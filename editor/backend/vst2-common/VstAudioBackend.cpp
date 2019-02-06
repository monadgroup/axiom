#include "VstAudioBackend.h"

#include "VstAdapter.h"

using namespace AxiomBackend;

VstAudioBackend::VstAudioBackend(AxiomBackend::VstAdapter &adapter, bool isSynth)
    : adapter(adapter), isSynth(isSynth) {}

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

    adapter.adapterUpdateIo();
}

std::string VstAudioBackend::getPortalLabel(size_t portalIndex) const {
    if ((ssize_t) portalIndex == midiInputPortal) return "1";

    auto audioInputIndex = audioInputs.portalParameterMap().find(portalIndex);
    if (audioInputIndex != audioInputs.portalParameterMap().end()) return std::to_string(audioInputIndex->second + 1);

    auto audioOutputIndex = audioOutputs.portalParameterMap().find(portalIndex);
    if (audioOutputIndex != audioOutputs.portalParameterMap().end())
        return std::to_string(audioOutputIndex->second + 1);

    auto automationIndex = automationInputs.portalParameterMap().find(portalIndex);
    if (automationIndex != automationInputs.portalParameterMap().end())
        return std::to_string(automationIndex->second + 1);

    return "?";
}

void VstAudioBackend::previewEvent(AxiomBackend::MidiEvent event) {
    if (midiInputPortal == -1) return;
    queueMidiEvent(0, (size_t) midiInputPortal, event);
}

void VstAudioBackend::automationValueChanged(size_t portalId, AxiomBackend::NumValue value) {
    auto mapIndex = automationInputs.portalParameterMap().find(portalId);
    if (mapIndex == automationInputs.portalParameterMap().end()) return;

    adapter.adapterSetParameter(mapIndex->second, value);
}

AxiomBackend::DefaultConfiguration VstAudioBackend::createDefaultConfiguration() {
    if (isSynth) {
        return DefaultConfiguration({DefaultPortal(PortalType::INPUT, PortalValue::MIDI, "Keyboard"),
                                     DefaultPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
    } else {
        return DefaultConfiguration({DefaultPortal(PortalType::INPUT, PortalValue::AUDIO, "Input"),
                                     DefaultPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Output")});
    }
}
