#pragma once

#include <optional>
#include <unordered_map>

#include "../AudioBackend.h"

struct VstParameter {
    uint64_t id;
    size_t portalIndex;
    AxiomBackend::NumValue **value;
    std::string name;
};

class AxiomVstPlugin;

class VstAudioBackend : public AxiomBackend::AudioBackend {
public:
    ssize_t midiInputPortal = 0;
    AxiomBackend::NumValue **outputPortal = nullptr;
    std::vector<std::optional<VstParameter>> parameters;
    std::unordered_map<size_t, size_t> portalParameterMap;

    explicit VstAudioBackend(AxiomVstPlugin *plugin);

    void handleConfigurationChange(const AxiomBackend::AudioConfiguration &configuration) override;

    bool doesSaveInternally() const override { return true; }

    void automationValueChanged(size_t portalId, AxiomBackend::NumValue value) override;

    bool canFiddleAutomation() const override { return true; }

private:
    void insertParameter(size_t insertIndex, VstParameter param);

    void pushParameter(VstParameter param);

    AxiomVstPlugin *plugin;
};
