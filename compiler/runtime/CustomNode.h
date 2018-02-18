#pragma once

#include "../codegen/Node.h"
#include "Node.h"
#include "HardControl.h"
#include "ErrorLog.h"

namespace MaximRuntime {

    class CustomNode : public Node {
    public:
        explicit CustomNode(Schematic *parent);

        std::string code() const { return _code; }

        void setCode(const std::string &code);

        void compile() override;

        std::vector<HardControl*> &controls() { return _controls; }

        ErrorLog const &errorLog() const { return _errorLog; }

    private:

        std::string _code = "";

        std::vector<HardControl*> _controls;

        MaximCodegen::Node _node;

        ErrorLog _errorLog;
    };

}
