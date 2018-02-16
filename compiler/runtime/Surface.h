#pragma once

#include <llvm/IR/Module.h>
#include <memory>
#include <vector>

#include "ControlGroup.h"
#include "../codegen/InstantiableFunction.h"
#include "Runtime.h"

namespace llvm {
    class Function;
}

namespace MaximCodegen {
    class MaximContext;
    class InstantiableFunction;
    class Node;
}

namespace MaximRuntime {

    class Node;

    class Surface {
    public:
        Surface *parent = nullptr;

        explicit Surface(MaximCodegen::MaximContext *context);

        void addNode(std::unique_ptr<Node> node);

        void removeNode(Node *node);

        void markAsDirty();

        MaximCodegen::InstantiableFunction *getFunction(Runtime *runtime);

        std::vector<ControlGroup*> &groups() { return _groups; }

        llvm::Module *module() { return &_module; }

        MaximCodegen::MaximContext *context() const { return _context; }

        bool isDirty() const { return _isDirty; }

    private:
        MaximCodegen::MaximContext *_context;
        llvm::Module _module;
        MaximCodegen::InstantiableFunction _instFunc;
        bool _isDirty = true;

        std::vector<std::unique_ptr<Node>> _nodes;
        std::vector<ControlGroup*> _groups;

        bool _hasHandle = false;
        Runtime::ModuleHandle _handle;
    };

}
