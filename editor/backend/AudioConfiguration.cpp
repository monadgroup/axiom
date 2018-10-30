#include "AudioConfiguration.h"

using namespace AxiomBackend;

ConfigurationPortal::ConfigurationPortal(size_t _key, uint64_t id, AxiomBackend::PortalType type,
                                         AxiomBackend::PortalValue value, std::string name)
    : _key(_key), id(id), type(type), value(value), name(std::move(name)) {}

bool ConfigurationPortal::operator==(const AxiomBackend::ConfigurationPortal &other) const {
    return type == other.type && value == other.value && name == other.name;
}

bool ConfigurationPortal::operator<(const AxiomBackend::ConfigurationPortal &other) const {
    return id < other.id;
}

AudioConfiguration::AudioConfiguration(std::vector<AxiomBackend::ConfigurationPortal> portals)
    : portals(std::move(portals)) {}

DefaultPortal::DefaultPortal(AxiomBackend::PortalType type, AxiomBackend::PortalValue value, std::string name)
    : type(type), value(value), name(std::move(name)) {}

DefaultConfiguration::DefaultConfiguration(std::vector<AxiomBackend::DefaultPortal> portals)
    : portals(std::move(portals)) {}
