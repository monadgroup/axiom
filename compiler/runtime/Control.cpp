#include "Control.h"

#include <cassert>
#include <unordered_set>
#include <queue>
#include <memory>

#include "ControlGroup.h"
#include "Runtime.h"
#include "Node.h"

using namespace MaximRuntime;

Control::Control(Node *node, std::string name, MaximCommon::ControlType type)
    : _node(node), _name(std::move(name)), _type(type) {
    auto newGroup = std::make_unique<ControlGroup>(_type, node->parentUnit());
    auto newGroupPtr = newGroup.get();
    node->parentUnit()->addControlGroup(std::move(newGroup));
    setGroup(newGroupPtr);
}

void Control::connectTo(Control *other) {
    assert(other->type() == type());

    if (other->group() == _group) return;

    _group->absorb(other->group());
    _connections.emplace(other);
    other->_connections.emplace(this);
    _group->schematic()->scheduleCompile();
}

void Control::disconnectFrom(Control *other) {
    if (other->group() != _group) return;

    _connections.erase(other);
    other->_connections.erase(this);

    // breadth-first search connections to find the two new groups
    // alternatively: the controls are still connected indirectly, so they're still in the same group
    std::unordered_set<Control*> visitedControls;
    std::queue<Control*> visitQueue;

    auto newGroup = std::make_unique<ControlGroup>(_type, _group->schematic());
    auto newGroupPtr = newGroup.get();
    _group->schematic()->addControlGroup(std::move(newGroup));

    visitedControls.emplace(this);
    visitQueue.emplace(this);
    while (!visitQueue.empty()) {
        auto visitControl = visitQueue.front();
        visitQueue.pop();

        for (const auto &connectedControl : visitControl->connections()) {
            auto index = visitedControls.find(connectedControl);
            if (index != visitedControls.end()) continue;

            visitedControls.emplace(connectedControl);
            visitQueue.push(connectedControl);

            // set the controls group to the new one
            connectedControl->setGroup(newGroupPtr);
        }
    }

    _group->schematic()->scheduleCompile();
}

void Control::setGroup(ControlGroup *newGroup) {
    if (_group) _group->removeControl(this);
    newGroup->addControl(this);
    _group = newGroup;
}

void Control::remove() {
    for (const auto &connectedNode : _connections) {
        disconnectFrom(connectedNode);
    }
    _group->removeControl(this);

    emit removed();
    emit cleanup();
}
