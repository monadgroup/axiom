#pragma once

#include <string>
#include <vector>

namespace AxiomBackend {

    enum class PortalType { INPUT, OUTPUT, AUTOMATION };

    enum class PortalValue { AUDIO, MIDI };

    class ConfigurationPortal {
    public:
        PortalType type;
        PortalValue value;
        std::string name;

        ConfigurationPortal(PortalType type, PortalValue value, std::string name);
    };

    class AudioConfiguration {
    public:
        std::vector<ConfigurationPortal> portals;

        explicit AudioConfiguration(std::vector<ConfigurationPortal> portals);
    };
}
