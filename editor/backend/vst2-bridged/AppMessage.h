#pragma once

namespace AxiomBackend {

    // Messages sent from the app to the VST for processing on the audio thread
    enum class AppAudioMessageType {
        GENERATE_DONE, // Samples have finished being generated
    };

    struct AppAudioGenerateDoneMessage {};

    union AppAudioMessageData {
        AppAudioGenerateDoneMessage generateDone;
    };

    struct AppAudioMessage {
        AppAudioMessageType type;
        AppAudioMessageData data;

        explicit AppAudioMessage(AppAudioMessageType type) : type(type) {}
    };

    // Messages sent from the app to the VST for processing on the GUI thread
    enum class AppGuiMessageType {
        SET_PARAMETER,          // Set a parameter value
        PARAMETER_VALUE,        // Parameter value response
        PARAMETER_LABEL,        // Parameter label response
        PARAMETER_DISPLAY,      // Parameter display response
        PARAMETER_NAME,         // Parameter name response
        CAN_AUTOMATE_PARAMETER, // Can automate parameter response
        FINISHED_SERIALIZE,     // Serialization is finished
        FINISHED_DESERIALIZE,   // Deserialization is finished
        UPDATE_IO,              // IO configuration has changed
    };

    struct AppGuiSetParameterMessage {
        int index;
        float value;
    };

    struct AppGuiParameterValueMessage {
        float value;
    };

    struct AppGuiParameterLabelMessage {
        char name[8];
    };

    struct AppGuiParameterDisplayMessage {
        char name[8];
    };

    struct AppGuiParameterNameMessage {
        char name[8];
    };

    struct AppGuiCanAutomateParameterMessage {
        bool canAutomate;
    };

    struct AppGuiFinishedSerializeMessage {
        uint32_t memoryId;
        uint64_t bufferSize;
    };

    struct AppGuiFinishedDeserializeMessage {};

    struct AppGuiUpdateIoMessage {
        uint32_t inputCount;
        uint32_t outputCount;
        uint32_t newMemoryId;
    };

    union AppGuiMessageData {
        AppGuiSetParameterMessage setParameter;
        AppGuiParameterValueMessage parameterValue;
        AppGuiParameterLabelMessage parameterLabel;
        AppGuiParameterDisplayMessage parameterDisplay;
        AppGuiParameterNameMessage parameterName;
        AppGuiCanAutomateParameterMessage canAutomateParameter;
        AppGuiFinishedSerializeMessage finishedSerialize;
        AppGuiFinishedDeserializeMessage finishedDeserialize;
        AppGuiUpdateIoMessage updateIo;
    };

    struct AppGuiMessage {
        AppGuiMessageType type;
        AppGuiMessageData data;

        explicit AppGuiMessage(AppGuiMessageType type) : type(type) {}
    };

}
