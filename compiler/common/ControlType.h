#pragma once

#include <string>

namespace MaximCommon {

    enum class ControlType {
        NUMBER,
        GRAPH,
        MIDI,
        ROLL,

        NUM_EXTRACT,
        MIDI_EXTRACT
    };

    std::string controlType2String(ControlType type);

}
