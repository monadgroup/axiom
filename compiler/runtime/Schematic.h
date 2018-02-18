#pragma once

#include "CompileUnit.h"

namespace MaximRuntime {

    class Node;

    class ControlGroup;

    class Schematic : public CompileUnit {
    public:
        explicit Schematic(Runtime *runtime, CompileUnit *parentUnit, size_t depth);

        size_t depth() const { return _depth; }

        CompileUnit *parentUnit() const override { return _parentUnit; }

        void compile() override;

        void deploy() override;

        std::vector<Node*> &nodes() { return _nodes; }

        void addControlGroup(std::unique_ptr<ControlGroup> group);

        std::unique_ptr<ControlGroup> removeControlGroup(ControlGroup *group);

    private:
        size_t _depth;

        CompileUnit *_parentUnit;

        std::vector<Node*> _nodes;

        std::vector<std::unique_ptr<ControlGroup>> _controlGroups;
    };

}
