#pragma once

#include <set>

#include "RuntimeUnit.h"
#include "../codegen/ComposableModuleClassMethod.h"

namespace MaximRuntime {

    class Node;

    class Runtime;

    class ControlGroup;

    class Surface : RuntimeUnit {
    public:
        Surface(Runtime *runtime, size_t depth);

        size_t depth() const { return _depth; }

        void scheduleGraphUpdate() { _needsGraphUpdate = true; }

        MaximCodegen::ComposableModuleClass *compile() override;

        void addNode(Node *node);

        void removeNode(Node *node);

        void addControlGroup(std::unique_ptr<ControlGroup> group);

        std::unique_ptr<ControlGroup> removeControlGroup(ControlGroup *group);

    private:
        llvm::Module _module;
        std::unique_ptr<MaximCodegen::ComposableModuleClass> _class;

        size_t _depth;
        bool _needsGraphUpdate = false;

        std::set<Node *> _nodes;
    };

}
