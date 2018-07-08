#pragma once

#include "../model/Value.h"
#include "AudioConfiguration.h"

namespace AxiomBackend {
    using NumValue = AxiomModel::NumValue;
    using NumForm = AxiomModel::FormType;
    using MidiValue = AxiomModel::MidiValue;
    using MidiEvent = AxiomModel::MidiEventValue;
    using MidiEventType = AxiomModel::MidiEventType;

    class AudioBackend {
    public:
        // Accessors for audio inputs and outputs
        // Note: the pointer returned is always valid as long as the portal ID is, however the target pointer may change
        // at any time from the UI thread.
        NumValue **getAudioPortal(size_t portalId) const;
        MidiValue **getMidiPortal(size_t portalId) const;

        // Queues a MIDI event to be input in a certain number of samples time. Should be called from the audio thread.
        void queueMidiEvent(size_t inSamples, size_t portalId, MidiEvent event);

        // Signals that you're about to start a batch of `generate` calls. The value returned signals the max number of
        // samples (i.e `generate` calls) until you should call `beginGenerate` again. This is used, for example, for
        // the internal queuing of MIDI events. Should be called from the audio thread.
        // Note: the return value of this function will _always_ be greater than 0.
        size_t beginGenerate();

        // Simulates the internal graph once. Inputs will be read as per their state before this call, and outputs will
        // be written to. Should be called from the audio thread. This call may block if the runtime is being rebuilt.
        void generate();

        // To be implemented by the audio backend, called from the UI thread when the IO configuration changes.
        // Note that this is not always called when the runtime is rebuilt, only if the rebuild results in a change in
        // configuration. Calls to `generate` will block while this method runs.
        virtual void handleConfigurationChange(const AudioConfiguration &configuration) = 0;

        // To be implemented by the audio backend, called from the UI thread when a new project is created to setup
        // a default configuration. `handleConfigurationChange` will still be called after the runtime is built for the
        // first time. The default implementation of this provides a configuration with one MIDI input and one audio
        // output.
        virtual AudioConfiguration createDefaultConfiguration();
    };
}
