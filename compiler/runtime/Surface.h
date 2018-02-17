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

        explicit Surface(Runtime *runtime);

        ~Surface();

        void addNode(Node *node);

        void removeNode(Node *node);

        void markAsDirty();

        MaximCodegen::InstantiableFunction *getFunction();

        std::vector<ControlGroup*> &groups() { return _groups; }

        llvm::Module *module() { return &_module; }

        Runtime *runtime() const { return _runtime; }

        MaximCodegen::MaximContext *context() const { return &_runtime->context; }

        bool isDirty() const { return _isDirty; }

    private:
        Runtime *_runtime;
        llvm::Module _module;
        MaximCodegen::InstantiableFunction _instFunc;
        bool _isDirty = true;

        std::vector<Node*> _nodes;
        std::vector<ControlGroup*> _groups;

        bool _hasHandle = false;
        Runtime::ModuleHandle _handle;
    };

}
