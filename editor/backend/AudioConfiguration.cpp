#include "AudioConfiguration.h"

using namespace AxiomBackend;

ConfigurationPortal::ConfigurationPortal(AxiomBackend::PortalType type, AxiomBackend::PortalValue value,
                                         std::string name)
    : type(type), value(value), name(std::move(name)) {}

bool ConfigurationPortal::operator==(const AxiomBackend::ConfigurationPortal &other) const {
    return type == other.type && value == other.value && name == other.name;
}

AudioConfiguration::AudioConfiguration(std::vector<AxiomBackend::ConfigurationPortal> portals)
    : portals(std::move(portals)) {}
