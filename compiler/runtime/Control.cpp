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

void Control::setExposer(MaximRuntime::Control *control) {
    if (control != _exposer) {
        _exposer = control;
        emit exposerChanged(control);
    }
}

void Control::exposerCleared(MaximRuntime::Control *exposer) {
    if (_exposer != exposer) return;
    _exposer = nullptr;
}

void Control::remove() {
    auto connections = std::set<Control *>(_connections);
    for (const auto &connectedNode : connections) {
        disconnectFrom(connectedNode);
    }
    _group->removeControl(this);

    onRemove();

    emit removed();
    emit cleanup();
}
