#pragma once

#include <string>
#include <memory>

namespace MaximCodegen {

    class ControlDeclaration;
    class Value;

    class Control {
    public:
        enum class Mode {
            UNKNOWN,
            INPUT,
            OUTPUT
        };

        explicit Control(ControlDeclaration *declaration);

        Value *getProperty(std::string name);

        bool setProperty(std::string name, std::unique_ptr<Value> value);

        void setMode(Mode mode);

    };

}
