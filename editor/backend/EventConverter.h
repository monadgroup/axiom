#pragma once

#include <optional>

#include "BackendDefines.h"

namespace AxiomBackend {

    std::optional<MidiEvent> convertFromMidi(int32_t event);
    int32_t convertToMidi(const MidiEvent &event);

}
