#include "AudioConfiguration.h"

using namespace AxiomBackend;

ConfigurationPortal::ConfigurationPortal(AxiomBackend::PortalType type, AxiomBackend::PortalValue value,
                                         std::string name)
    : type(type), value(value), name(std::move(name)) {}

AudioConfiguration::AudioConfiguration(std::vector<AxiomBackend::ConfigurationPortal> portals)
    : portals(std::move(portals)) {}
