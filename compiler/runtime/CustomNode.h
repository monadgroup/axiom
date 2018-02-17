#pragma once

#include "Node.h"
#include "ErrorLog.h"
#include "../codegen/Node.h"
#include "Runtime.h"

namespace llvm {
    class Function;
}

namespace MaximCodegen {
    class Node;
}

namespace MaximRuntime {

    class Control;

    class CustomNode : public Node {
    public:
        explicit CustomNode(Surface *surface);

        ~CustomNode();

        ErrorLog compile(std::string content);

        std::vector<std::unique_ptr<Control>> &controls() override { return _controls; }

        MaximCodegen::Node *getFunction() override;

    private:
        Surface *_surface;
        llvm::Module _module;

        MaximCodegen::Node _node;

        std::vector<std::unique_ptr<Control>> _controls;

        bool _hasHandle = false;
        Runtime::ModuleHandle _handle;

        void updateControls();
    };

}
