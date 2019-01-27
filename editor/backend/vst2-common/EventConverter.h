#pragma once

#include <public.sdk/source/vst2.x/audioeffectx.h>
#include <optional>

#include "../BackendDefines.h"

namespace AxiomBackend {

    std::optional<MidiEvent> convertFromVst(VstMidiEvent *event);

}
