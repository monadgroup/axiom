#include "Control.h"

#include <cassert>
#include <unordered_set>
#include <queue>
#include <iostream>

#include "Node.h"
#include "ControlGroup.h"
#include "Surface.h"
#include "../codegen/Scope.h"

using namespace MaximRuntime;

Control::Control(Node *node)
    : _node(node) {
}

void Control::finish() {
    auto newGroup = std::make_unique<ControlGroup>(_node->surface(), type());
    auto newGroupPtr = newGroup.get();
    _node->surface()->addControlGroup(std::move(newGroup));
    setGroup(newGroupPtr);
}

void Control::setGroup(ControlGroup *group) {
    if (_group) _group->removeControl(this);
    group->addControl(this);
    _group = group;
}

void Control::connectTo(Control *other) {
    //assert(other->type() == type());

    if (other->group() == _group) return;

    _group->absorb(other->group());
    _connections.emplace(other);
    other->_connections.emplace(this);
    _group->surface()->scheduleGraphUpdate();
}

void Control::disconnectFrom(Control *other) {
    if (other->group() != _group) return;

    _connections.erase(other);
    other->_connections.erase(this);

    // breadth-first search connections to find the two new groups
    // alternatively: the controls are still connected indirectly, so they're still in the same group
    std::unordered_set<Control *> visitedControls;
    std::queue<Control *> visitQueue;

    auto newGroup = std::make_unique<ControlGroup>(_group->surface(), type());
    auto newGroupPtr = newGroup.get();
    _group->surface()->addControlGroup(std::move(newGroup));

    visitedControls.emplace(this);
    visitQueue.emplace(this);
    while (!visitQueue.empty()) {
        auto visitControl = visitQueue.front();
        visitQueue.pop();

        // set the controls group to the new one
        visitControl->setGroup(newGroupPtr);

        for (const auto &connectedControl : visitControl->connections()) {
            auto index = visitedControls.find(connectedControl);
            if (index != visitedControls.end()) continue;

            visitedControls.emplace(connectedControl);
            visitQueue.push(connectedControl);
        }
    }

    if (newGroupPtr->controls().empty()) {
        _group->surface()->removeControlGroup(newGroupPtr);
    }

    _group->surface()->scheduleGraphUpdate();
}

void Control::remove() {
    auto connections = std::set<Control *>(_connections);
    for (const auto &connectedNode : connections) {
        disconnectFrom(connectedNode);
    }
    _group->removeControl(this);

    emit removed();
    emit cleanup();
}

bool Control::exposed() const {
    // todo
    return false;
}
