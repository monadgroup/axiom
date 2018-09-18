#include "VstAudioBackend.h"

#include <unordered_map>
#include <unordered_set>

using namespace AxiomBackend;

void VstAudioBackend::handleConfigurationChange(const AxiomBackend::AudioConfiguration &configuration) {
    midiInputPortal = -1;
    outputPortal = nullptr;

    // build an map of the portal ID to parameter index
    std::unordered_map<uint64_t, size_t> parameterIndexMap;
    for (size_t parameterIndex = 0; parameterIndex < parameters.size(); parameterIndex++) {
        auto &parameter = parameters[parameterIndex];
        if (parameter) {
            parameterIndexMap.emplace(parameter->id, parameterIndex);
        }
    }

    parameters.clear();
    std::unordered_set<size_t> takenIndices;
    std::vector<VstParameter> queuedParameters;

    // we want the first MIDI input, audio output, and every automation parameter
    for (size_t i = 0; i < configuration.portals.size(); i++) {
        const auto &portal = configuration.portals[i];
        if (midiInputPortal == -1 && portal.type == PortalType::INPUT && portal.value == PortalValue::MIDI) {
            midiInputPortal = i;
        } else if (outputPortal == nullptr && portal.type == PortalType::OUTPUT && portal.value == PortalValue::AUDIO) {
            outputPortal = getAudioPortal(i);
        } else if (portal.type == PortalType::AUTOMATION && portal.value == PortalValue::AUDIO) {
            auto previousParameterIndex = parameterIndexMap.find(portal.id);

            VstParameter insertParam = {portal.id, getAudioPortal(i), portal.name};

            if (previousParameterIndex != parameterIndexMap.end()) {
                takenIndices.emplace(previousParameterIndex->second);
                insertParameter(previousParameterIndex->second, std::move(insertParam));
            } else {
                // queue the parameter to be added after all "taken" ones are
                queuedParameters.push_back(std::move(insertParam));
            }
        }
    }

    // now go through and insert all remaining ones
    for (auto &queuedParam : queuedParameters) {
        pushParameter(std::move(queuedParam));
    }
}

void VstAudioBackend::insertParameter(size_t insertIndex, VstParameter param) {
    while (parameters.size() < insertIndex) parameters.emplace_back(std::nullopt);
    parameters[insertIndex] = std::move(param);
}

void VstAudioBackend::pushParameter(VstParameter param) {
    // loop until we find an available index
    size_t nextIndex = 0;
    while (nextIndex < parameters.size() && parameters[nextIndex]) {
        nextIndex++;
    }

    insertParameter(nextIndex, std::move(param));
}
