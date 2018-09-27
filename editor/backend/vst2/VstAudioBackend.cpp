#include "VstAudioBackend.h"

#include <unordered_map>
#include <unordered_set>

#include "AxiomVstPlugin.h"

using namespace AxiomBackend;

VstAudioBackend::VstAudioBackend(AxiomVstPlugin *plugin) : plugin(plugin) {}

void VstAudioBackend::handleConfigurationChange(const AxiomBackend::AudioConfiguration &configuration) {
    std::vector<AxiomBackend::NumParameter> newAudioInputs;
    std::vector<AxiomBackend::NumParameter> newAudioOutputs;
    std::vector<AxiomBackend::NumParameter> newAutomationInputs;

    midiInputPortal = -1;

    for (size_t i = 0; i < configuration.portals.size(); i++) {
        const auto &portal = configuration.portals[i];
        switch (portal.type) {
        case PortalType::INPUT:
            if (portal.value == PortalValue::AUDIO) {
                newAudioInputs.emplace_back(portal.id, i, getAudioPortal(i), portal.name);
            } else if (portal.value == PortalValue::MIDI && midiInputPortal == -1) {
                midiInputPortal = i;
            }
            break;
        case PortalType::OUTPUT:
            if (portal.value == PortalValue::AUDIO) {
                newAudioOutputs.emplace_back(portal.id, i, getAudioPortal(i), portal.name);
            }
            break;
        case PortalType::AUTOMATION:
            if (portal.value == PortalValue::AUDIO) {
                newAutomationInputs.emplace_back(portal.id, i, getAudioPortal(i), portal.name);
            }
        }
    }

    audioInputs.setParameters(std::move(newAudioInputs));
    audioOutputs.setParameters(std::move(newAudioOutputs));
    automationInputs.setParameters(std::move(newAutomationInputs));

    plugin->backendUpdateIo();
}

void VstAudioBackend::automationValueChanged(size_t portalId, AxiomBackend::NumValue value) {
    auto mapIndex = automationInputs.portalParameterMap().find(portalId);
    if (mapIndex == automationInputs.portalParameterMap().end()) return;

    plugin->backendSetParameter(mapIndex->second, value);
}

DefaultConfiguration VstAudioBackend::createDefaultConfiguration() {
#ifdef AXIOM_VST2_IS_SYNTH
    return DefaultConfiguration({DefaultPortal(PortalType::INPUT, PortalValue::MIDI, "Keyboard"),
                                 DefaultPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
#else
    return DefaultConfiguration({DefaultPortal(PortalType::INPUT, PortalValue::AUDIO, "Input"),
                                 DefaultPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Output")});
#endif
}
