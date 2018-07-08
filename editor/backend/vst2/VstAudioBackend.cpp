#include "VstAudioBackend.h"

using namespace AxiomBackend;

void VstAudioBackend::handleConfigurationChange(const AxiomBackend::AudioConfiguration &configuration) {
    midiInputPortal = -1;
    outputPortal = nullptr;
    parameters.clear();

    // we want the first MIDI input, audio output, and every automation parameter
    for (size_t i = 0; i < configuration.portals.size(); i++) {
        const auto &portal = configuration.portals[i];
        if (midiInputPortal == -1 && portal.type == PortalType::INPUT && portal.value == PortalValue::MIDI) {
            midiInputPortal = i;
        } else if (outputPortal == nullptr && portal.type == PortalType::OUTPUT && portal.value == PortalValue::AUDIO) {
            outputPortal = getAudioPortal(i);
        } else if (portal.type == PortalType::AUTOMATION && portal.value == PortalValue::AUDIO) {
            parameters.push_back({getAudioPortal(i), portal.name});
        }
    }
}
