#include "Schematic.h"

#include "Node.h"
#include "ControlGroup.h"
#include "Runtime.h"

using namespace MaximRuntime;

Schematic::Schematic(Runtime *runtime, CompileUnit *parentUnit, size_t depth)
    : CompileUnit(runtime), _depth(depth), _parentUnit(parentUnit) {

}

Schematic::~Schematic() = default;

void Schematic::compile() {
    instFunc()->reset();

    for (const auto &node : _nodes) {
        if (node->needsCompile()) node->compile();
    }

    // instantiate nodes
    // todo: calculate ordering?
    // todo: instantiate extractor nodes several times
    for (const auto &node : _nodes) {
        instFunc()->addInstantiable(node->instFunc());
    }

    // instantiate control groups
    for (const auto &group : _controlGroups) {
        instFunc()->addInstantiable(group.get());
    }

    instFunc()->complete();

    CompileUnit::compile();
}

void Schematic::deploy() {
    for (const auto &node : _nodes) {
        if (node->needsCompile()) node->compile();
        if (node->needsDeploy()) node->deploy();
    }

    CompileUnit::deploy();
}

void Schematic::addNode(Node *node) {
    assert(node->parentUnit() == this);

    _nodes.emplace(node);
}

void Schematic::removeNode(Node *node) {
    _nodes.erase(node);
    scheduleCompile();
}

void Schematic::addControlGroup(std::unique_ptr<ControlGroup> group) {
    _controlGroups.push_back(std::move(group));
    scheduleCompile();
}

std::unique_ptr<ControlGroup> Schematic::removeControlGroup(ControlGroup *group) {
    scheduleCompile();

    for (auto i = _controlGroups.begin(); i < _controlGroups.end(); i++) {
        auto &indexedGroup = *i;

        if (indexedGroup.get() == group) {
            auto movedGroup = std::move(indexedGroup);
            _controlGroups.erase(i);
            return movedGroup;
        }
    }

    assert(false);
    throw;
}
