#pragma once

#include <set>
#include <unordered_map>

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

        void scheduleChildUpdate();

        bool needsGraphUpdate() const;

        GeneratableModuleClass *compile();

        void addNode(std::unique_ptr<Node> node);

        virtual void removeNode(Node *node);

        void pullMethods() override;

        void pullMethods(MaximCodegen::ModuleClassMethod *getterMethod,
                         MaximCodegen::ModuleClassMethod *destroyMethod) override;

        void *updateCurrentPtr(void *parentCtx) override;

        std::unordered_map<ControlGroup *, size_t> const &groupPtrIndexes() const { return _groupPtrIndexes; };

        virtual void addExitNodes(std::set<Node *> &queue);

        void saveValue() override;

        void restoreValue() override;

        MaximCodegen::ModuleClass *moduleClass() override { return _class.get(); }

    private:
        std::unique_ptr<GeneratableModuleClass> _class;

        size_t _depth;
        bool _needsGraphUpdate = false;

        std::set<std::unique_ptr<Node>> _nodes;

        std::vector<std::unique_ptr<ControlGroup>> _controlGroups;

        std::unordered_map<ControlGroup *, size_t> _groupPtrIndexes;
    };

}
