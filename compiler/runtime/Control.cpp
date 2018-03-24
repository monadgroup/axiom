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

void Control::setGroup(ControlGroup *group) {
    if (_group) _group->removeControl(this);
    if (group) group->addControl(this);
    _group = group;
}

void Control::connectTo(MaximRuntime::Control *other) {
    _connections.emplace(other);
    other->_connections.emplace(this);
    _node->surface()->scheduleGraphUpdate();
}

void Control::disconnectFrom(MaximRuntime::Control *other) {
    _connections.erase(other);
    other->_connections.erase(this);
    _node->surface()->scheduleGraphUpdate();
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

Control *Control::exposer() const {
    // todo
    return nullptr;
}
