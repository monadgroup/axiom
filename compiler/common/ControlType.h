#pragma once

#include <string>

namespace MaximCommon {

    enum class ControlType {
        NUMBER = 1 << 0,
        GRAPH = 1 << 1,
        MIDI = 1 << 2,
        ROLL = 1 << 3,

        NUM_EXTRACT = 1 << 4,
        MIDI_EXTRACT = 1 << 5,

        EXTRACT = NUM_EXTRACT | MIDI_EXTRACT
    };

    std::string controlType2String(ControlType type);

}
