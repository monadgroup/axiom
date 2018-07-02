#pragma once

#include <string>

namespace MaximCompiler {

    enum class ControlType {
        Audio,
        Graph,
        Midi,
        Roll,
        Scope,
        AudioExtract,
        MidiExtract
    };

    class ControlRef {
    public:
        explicit ControlRef(void *handle);

        void *get() const { return handle; }

        std::string getName() const;

        ControlType getType() const;

        bool getIsWritten() const;

        bool getIsRead() const;

    private:
        void *handle;
    };

}
