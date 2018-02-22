#pragma once

#include <set>

#include "CompileUnit.h"

namespace MaximRuntime {

    class Node;

    class ControlGroup;

    class Schematic : public CompileUnit {
    public:
        explicit Schematic(Runtime *runtime, CompileUnit *parentUnit, size_t depth);

        ~Schematic() override;

        size_t depth() const { return _depth; }

        CompileUnit *parentUnit() const override { return _parentUnit; }

        void compile() override;

        void deploy() override;

        void addNode(Node *node);

        void removeNode(Node *node);

        void *updateCurrentPtr(void *parentCtx) override;

        std::set<Node*> &nodes() { return _nodes; }

        void addControlGroup(std::unique_ptr<ControlGroup> group);

        std::unique_ptr<ControlGroup> removeControlGroup(ControlGroup *group);

    private:
        size_t _depth;

        CompileUnit *_parentUnit;

        std::set<Node*> _nodes;

        std::vector<std::unique_ptr<ControlGroup>> _controlGroups;
    };

}
