#include "VstAudioBackend.h"

#include <unordered_map>
#include <unordered_set>

#include "AxiomVstPlugin.h"

using namespace AxiomBackend;

VstAudioBackend::VstAudioBackend(AxiomVstPlugin *plugin) : plugin(plugin) {}

void VstAudioBackend::handleConfigurationChange(const AxiomBackend::AudioConfiguration &configuration) {
    midiInputPortal = -1;
    outputPortal = nullptr;

    std::vector<AxiomBackend::NumParameter> newParameters;

    // we want the first MIDI input, audio output, and every automation parameter
    for (size_t i = 0; i < configuration.portals.size(); i++) {
        const auto &portal = configuration.portals[i];
        if (midiInputPortal == -1 && portal.type == PortalType::INPUT && portal.value == PortalValue::MIDI) {
            midiInputPortal = i;
        } else if (outputPortal == nullptr && portal.type == PortalType::OUTPUT && portal.value == PortalValue::AUDIO) {
            outputPortal = getAudioPortal(i);
        } else if (portal.type == PortalType::AUTOMATION && portal.value == PortalValue::AUDIO) {
            newParameters.emplace_back(portal.id, i, getAudioPortal(i), portal.name);
        }
    }

    parameters.setParameters(std::move(newParameters));
}

void VstAudioBackend::automationValueChanged(size_t portalId, AxiomBackend::NumValue value) {
    auto mapIndex = parameters.portalParameterMap().find(portalId);
    if (mapIndex == parameters.portalParameterMap().end()) return;

    plugin->backendSetParameter(mapIndex->second, value);
}
