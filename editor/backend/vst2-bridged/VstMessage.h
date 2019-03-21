#pragma once

namespace AxiomBackend {

    // Messages sent from the VST to the app for processing on the audio thread
    enum class VstAudioMessageType {
        GENERATE, // Generate samples into the output buffer
        PUSH_MIDI_EVENT, // Add an event to the queue
        SET_SAMPLE_RATE, // Set the sample rate
        SET_BPM, // Set the BPM

        EXIT, // Stop audio processing and exit (sent along with a VstGuiMessage EXIT)
    };

    struct VstAudioGenerateMessage {
        uint8_t sampleCount;
    };

    struct VstAudioPushMidiEventMessage {
        int deltaSamples;
        uint8_t event;
        uint8_t channel;
        uint8_t note;
        uint8_t param;
    };

    struct VstAudioSetSampleRateMessage {
        float sampleRate;
    };

    struct VstAudioSetBpmMessage {
        float bpm;
    };

    struct VstAudioExit {};

    union VstAudioMessageData {
        VstAudioGenerateMessage generate;
        VstAudioPushMidiEventMessage pushMidiEvent;
        VstAudioSetSampleRateMessage setSampleRate;
        VstAudioSetBpmMessage setBpm;
        VstAudioExit exit;
    };

    struct VstAudioMessage {
        VstAudioMessageType type;
        VstAudioMessageData data;

        explicit VstAudioMessage(VstAudioMessageType type) : type(type) {}
    };

    // Messages sent from the VST to the app for processing on the GUI thread
    enum class VstGuiMessageType {
        SHOW, // Show the editor window
        HIDE, // Hide the editor window
        SET_PARAMETER, // Set a parameter value
        GET_PARAMETER, // Get a parameter value
        GET_PARAMETER_LABEL, // Get the label of a parameter
        GET_PARAMETER_DISPLAY, // Get the display of a parameter
        GET_PARAMETER_NAME, // Get the name of a parameter
        GET_CAN_AUTOMATE_PARAMETER, // Get if a parameter can be automated
        BEGIN_SERIALIZE, // Start serializing the project
        END_SERIALIZE, // End serializing the project
        BEGIN_DESERIALIZE, // Start deserializing a project

        EXIT, // Exit the application
    };

    struct VstGuiShowMessage {};

    struct VstGuiHideMessage {};

    struct VstGuiSetParameterMessage {
        int index;
        float value;
    };

    struct VstGuiGetParameterMessage {
        int index;
    };

    struct VstGuiGetParameterLabelMessage {
        int index;
    };

    struct VstGuiGetParameterDisplayMessage {
        int index;
    };

    struct VstGuiGetParameterNameMessage {
        int index;
    };

    struct VstGuiGetCanAutomateParameterMessage {
        int index;
    };

    struct VstGuiBeginSerializeMessage {};

    struct VstGuiEndSerializeMessage {};

    struct VstGuiBeginDeserializeMessage {
        uint32_t memoryId;
        uint64_t bufferSize;
    };

    struct VstGuiExit {};

    union VstGuiMessageData {
        VstGuiShowMessage show;
        VstGuiHideMessage hide;
        VstGuiSetParameterMessage setParameter;
        VstGuiGetParameterMessage getParameter;
        VstGuiGetParameterLabelMessage getParameterLabel;
        VstGuiGetParameterDisplayMessage getParameterDisplay;
        VstGuiGetParameterNameMessage getParameterName;
        VstGuiGetCanAutomateParameterMessage getCanAutomateParameter;
        VstGuiBeginSerializeMessage beginSerialize;
        VstGuiEndSerializeMessage endSerialize;
        VstGuiBeginDeserializeMessage beginDeserialize;
        VstGuiExit exit;
    };

    struct VstGuiMessage {
        VstGuiMessageType type;
        VstGuiMessageData data;

        explicit VstGuiMessage(VstGuiMessageType type) : type(type) {}
    };
}
