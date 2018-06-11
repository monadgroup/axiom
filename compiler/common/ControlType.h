#pragma once

#include <string>

namespace MaximCommon {

    enum class ControlType {
        NUMBER = 1 << 0,
        GRAPH = 1 << 1,
        MIDI = 1 << 2,
        ROLL = 1 << 3,
        SCOPE = 1 << 4,

        NUM_EXTRACT = 1 << 5,
        MIDI_EXTRACT = 1 << 6
    };

    std::string controlType2String(ControlType type);

}
