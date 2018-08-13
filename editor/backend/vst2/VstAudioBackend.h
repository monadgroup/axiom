#pragma once
#include "../AudioBackend.h"

struct VstParameter {
    AxiomBackend::NumValue **value;
    std::string name;
};

class VstAudioBackend : public AxiomBackend::AudioBackend {
public:
    ssize_t midiInputPortal = 0;
    AxiomBackend::NumValue **outputPortal = nullptr;
    std::vector<VstParameter> parameters;

    void handleConfigurationChange(const AxiomBackend::AudioConfiguration &configuration) override;

    bool doesSaveInternally() const override { return true; }
};
