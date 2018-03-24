#pragma once

#include <set>

#include "ModuleRuntimeUnit.h"
#include "ControlGroup.h"
#include "GeneratableModuleClass.h"
#include "../codegen/ComposableModuleClassMethod.h"

namespace MaximRuntime {

    class Node;

    class Runtime;

    class ControlGroup;

    class GeneratableModuleClass;

    class Surface : public ModuleRuntimeUnit {
    public:
        Surface(Runtime *runtime, size_t depth);

        size_t depth() const { return _depth; }

        void scheduleGraphUpdate();

        GeneratableModuleClass *compile();

        void addNode(Node *node);

        void removeNode(Node *node);

        void pullGetterMethod() override;

        void *updateCurrentPtr(void *parentCtx) override;

        virtual void addExitNodes(std::set<Node *> &queue);

    private:
        std::unique_ptr<GeneratableModuleClass> _class;

        size_t _depth;
        bool _needsGraphUpdate = false;

        std::set<Node *> _nodes;

        std::vector<std::unique_ptr<ControlGroup>> _controlGroups;
    };

}
