#pragma once

#include "../codegen/Node.h"
#include "Node.h"
#include "HardControl.h"
#include "ErrorLog.h"

namespace MaximRuntime {

    class CustomNode : public QObject, public Node {
        Q_OBJECT

    public:
        explicit CustomNode(Schematic *parent);

        ~CustomNode();

        std::string code() const { return _code; }

        void setCode(const std::string &code);

        void compile() override;

        std::vector<std::unique_ptr<HardControl>> &controls() { return _controls; }

        ErrorLog const &errorLog() const { return _errorLog; }

    signals:

        void controlAdded(HardControl *control);

    private:

        std::string _code = "";

        std::vector<std::unique_ptr<HardControl>> _controls;

        MaximCodegen::Node _node;

        ErrorLog _errorLog;

        void updateControls();
    };

}
