#pragma once

#include <optional>

#include "../AudioBackend.h"

struct VstParameter {
    uint64_t id;
    AxiomBackend::NumValue **value;
    std::string name;
};

class VstAudioBackend : public AxiomBackend::AudioBackend {
public:
    ssize_t midiInputPortal = 0;
    AxiomBackend::NumValue **outputPortal = nullptr;
    std::vector<std::optional<VstParameter>> parameters;

    void handleConfigurationChange(const AxiomBackend::AudioConfiguration &configuration) override;

    bool doesSaveInternally() const override { return true; }

private:
    void insertParameter(size_t insertIndex, VstParameter param);

    void pushParameter(VstParameter param);
};
