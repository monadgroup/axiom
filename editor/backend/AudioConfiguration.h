#pragma once

#include <string>
#include <vector>

namespace AxiomBackend {

    enum class PortalType { INPUT, OUTPUT, AUTOMATION };

    enum class PortalValue { AUDIO, MIDI };

    class ConfigurationPortal {
    public:
        size_t _key;

        uint64_t id;
        PortalType type;
        PortalValue value;
        std::string name;

        ConfigurationPortal(size_t _key, uint64_t id, PortalType type, PortalValue value, std::string name);

        bool operator==(const ConfigurationPortal &other) const;

        bool operator<(const ConfigurationPortal &other) const;
    };

    class AudioConfiguration {
    public:
        std::vector<ConfigurationPortal> portals;

        explicit AudioConfiguration(std::vector<ConfigurationPortal> portals);
    };

    class DefaultPortal {
    public:
        PortalType type;
        PortalValue value;
        std::string name;

        DefaultPortal(PortalType type, PortalValue value, std::string name);
    };

    class DefaultConfiguration {
    public:
        std::vector<DefaultPortal> portals;

        explicit DefaultConfiguration(std::vector<DefaultPortal> portals);
    };
}
