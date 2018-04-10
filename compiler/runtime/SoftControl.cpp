#include <queue>
#include "SoftControl.h"

#include "GroupNode.h"

using namespace MaximRuntime;

SoftControl::SoftControl(MaximRuntime::GroupNode *node, MaximRuntime::Control *forward)
    : Control(node), _node(node), _forward(forward) {
    forward->setExposer(this);

    connect(forward, &Control::removed,
            this, &SoftControl::remove);
    connect(forward, &Control::exposerChanged,
            this, &SoftControl::remove);
}

std::unique_ptr<SoftControl> SoftControl::create(MaximRuntime::GroupNode *node, MaximRuntime::Control *forward) {
    return std::make_unique<SoftControl>(node, forward);
}

std::string SoftControl::name() const {
    return _forward->name();
}

MaximCodegen::Control *SoftControl::type() const {
    return _forward->type();
}

bool SoftControl::writtenTo() const {
    return _forward->writtenTo();
}

bool SoftControl::readFrom() const {
    return _forward->readFrom();
}

void SoftControl::addInternallyLinkedControls(std::set<Control *> &controls) {
    auto controlQueue = std::queue<Control *>();
    auto internalConnections = _forward->connections();
    for (const auto &connection : internalConnections) controlQueue.push(connection);

    while (!controlQueue.empty()) {
        auto control = controlQueue.front();
        controlQueue.pop();
        if (control->node() != _forward->node() || !control->exposer()) continue;

        controls.emplace(control->exposer());
        control->addInternallyLinkedControls(controls);

        auto controlConnections = control->connections();
        for (const auto &connection : controlConnections) {
            controlQueue.push(connection);
        }
    }
}

void SoftControl::onRemove() {
    _forward->exposerCleared(this);
}
