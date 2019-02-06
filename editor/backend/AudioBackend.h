#pragma once

#include <QtCore/QByteArray>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>

#include "AudioConfiguration.h"
#include "BackendDefines.h"

class AxiomEditor;

namespace AxiomBackend {
    class AudioBackend;

    // An RAII handler for generating samples into a buffer.
    // While this object is alive, the runtime is guaranteed to be locked, and the FPU state will be correct.
    // You shouldn't call `generate` more times than `maxGenerateCount`. Once the count has expired, delete this object
    // and request a new one with `beginGenerate` on the audio backend.
    // ONLY call `generate` from the thread you requested the context from!
    class GenerateContext {
        friend class AudioBackend;

    private:
        GenerateContext(uint64_t count, AudioBackend *backend);

    public:
        ~GenerateContext();

        // The max number of samples (i.e `generate` calls) until you should request a new context. This is used, for
        // example, for the internal queueing of MIDI events.
        // Note: the return value of this function will _always_ be greater than 0. This value will not change after
        // the class has been created.
        uint64_t maxGenerateCount() const { return _maxGenerateCount; }

        // Simulates the internal graph once. Inputs will be read as per their state before this call, and outputs will
        // be written to.
        void generate();

    private:
        AudioBackend *backend;
        uint64_t _maxGenerateCount;
        std::lock_guard<std::mutex> runtimeLock;
        unsigned int beforeFpuState;
    };

    class AudioBackend {
        friend class GenerateContext;

    public:
        // Accessors for audio inputs and outputs
        // Note: the pointer returned is always valid as long as the portal ID is, however the target pointer may change
        // at any time from the UI thread.
        NumValue **getAudioPortal(size_t portalId) const;
        MidiValue **getMidiPortal(size_t portalId) const;

        // Sets the BPM or sample rate on the runtime. Should be called from the audio thread.
        void setBpm(float bpm);
        void setSampleRate(float sampleRate);

        // Formats a form or number.
        static const char *formatNumForm(float testValue, NumForm form);
        static std::string formatNum(NumValue value, bool includeLabel);

        // Finds a file by the specified name in one of the data paths, or returns an empty string.
        static std::string findDataFile(const std::string &name);

        // Returns the main writable data path, guaranteed to exist.
        static std::string getDataPath();

        // Serializes or deserializes the current open project. Use this for saving/loading the project from a DAW
        // project file.
        QByteArray serialize(std::optional<std::function<void(QDataStream &)>> serializeCustomCallback = std::nullopt);
        void deserialize(
            QByteArray *data,
            std::optional<std::function<void(QDataStream &, uint32_t)>> deserializeCustomCallback = std::nullopt);

        // Queues a MIDI event to be input in a certain number of samples time. Should be called from the audio thread.
        // You should call clearMidi after the first generated sample (at least) to clear the MIDI portals that had
        // data queued.
        void queueMidiEvent(uint64_t deltaFrames, size_t portalId, MidiEvent event);
        void clearMidi(size_t portalId);

        // Clears all pressed MIDI keys. Should be called from the audio thread.
        void clearNotes(size_t portalId);

        // Signals that you're about to start a batch of `generate` calls. May block if the runtime is being rebuilt,
        // but as long as the returned value exists, generation calls are safe.
        GenerateContext beginGenerate();

        // To be implemented by the audio backend, called from the UI thread when the IO configuration changes.
        // Note that this is not always called when the runtime is rebuilt, only if the rebuild results in a change in
        // configuration. The runtime will be locked while in this method.
        virtual void handleConfigurationChange(const AudioConfiguration &configuration) = 0;

        // To be implemented by the audio backend, called from the UI thread when a new project is created to setup
        // a default configuration. `handleConfigurationChange` will still be called after the runtime is built for the
        // first time.
        virtual DefaultConfiguration createDefaultConfiguration() = 0;

        // To be implemented by the audio backend, called from the UI thread to determine if save dialogues should be
        // shown. This could be called at any time.
        virtual bool doesSaveInternally() const = 0;

        // To be implemented by the audio backend, called from the UI thread to determine a label for portal nodes.
        virtual std::string getPortalLabel(size_t portalIndex) const = 0;

        // To be implemented by the audio backend, called from the UI thread when the user presses or releases a key
        // corresponding to a MIDI note. The default implementation does nothing.
        virtual void previewEvent(MidiEvent event);

        // To be implemented by the audio backend, called from the UI thread when an automation portal value changes.
        // This is not called every sample a value changes, but every "update cycle" (roughly every 16 milliseconds)
        // where the value changed from the last cycle. The default implementation does nothing.
        virtual void automationValueChanged(size_t portalIndex, NumValue value);

        // To be implemented by the audio backend, called from the UI thread to determine if the user is able to
        // "fiddle" automation portals. This should return true on backends where a potential host needs a value change
        // to interact with the portal. The default implementation returns false.
        virtual bool canFiddleAutomation() const;

        // Called internally. Not stable APIs.
        void setEditor(AxiomEditor *editor) { _editor = editor; }
        void internalUpdateConfiguration();
        size_t internalRemapPortal(uint64_t id);

    private:
        struct QueuedEvent {
            uint64_t deltaFrames;
            size_t portalId;
            MidiEvent event;
        };

        bool hasCurrent = false;
        std::vector<ConfigurationPortal> currentPortals;

        AxiomEditor *_editor;
        std::vector<void *> portalValues;

        // todo: use a circular buffer instead of a deque here
        std::deque<QueuedEvent> queuedEvents;
        size_t generatedSamples = 0;
    };
}
