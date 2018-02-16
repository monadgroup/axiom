#include "Control.h"

#include <unordered_set>
#include <queue>

#include "ControlGroup.h"
#include "Node.h"
#include "Surface.h"
#include "ExposedControl.h"

using namespace MaximRuntime;

Control::Control(Node *node, std::string name, MaximCodegen::Control *control)
    : _node(node), _name(std::move(name)), _type(control->type()), _dir(control->direction) {
    _group = std::make_shared<ControlGroup>(this);
}

std::unique_ptr<Control> Control::create(Node *node, std::string name, MaximCodegen::Control *control) {
    return std::make_unique<Control>(node, name, control);
}

void Control::setDirection(MaximCommon::ControlDirection dir) {
    dir = _dir;
    if (dir == MaximCommon::ControlDirection::OUT && !_group->writer()) {
        _group->setWriter(this);
        _node->surface()->markAsDirty();
    } else if (_group->writer() == this) {
        _group->findWriter(this);
    }

    if (_exposer) _exposer->setDirection(dir);
}

void Control::setExposer(ExposedControl *exposer) {
    _exposer = exposer;
    if (exposer) {
        _group->setExposed(true);
    } else {
        _group->updateExposed();
    }
}

void Control::connectTo(Control *control) {
    assert(type() == control->type());

    _connections.push_back(control);
    control->connections().push_back(this);

    // merge other controls group into this one
    auto otherGroup = control->group();
    for (const auto &otherControl : otherGroup->controls()) {
        _group->controls().push_back(otherControl);
        otherControl->setGroup(_group);
    }
    if (!_group->writer()) _group->setWriter(otherGroup->writer());

    _group->setExposed(_group->isExposed() || otherGroup->isExposed());

    _node->surface()->markAsDirty();
}

void Control::disconnectFrom(Control *control) {
    auto myConIter = std::find(_connections.begin(), _connections.end(), control);
    if (myConIter == _connections.end()) return;
    _connections.erase(myConIter);

    auto otherConIter = std::find(control->connections().begin(), control->connections().end(), this);
    if (otherConIter != control->connections().end()) control->connections().erase(otherConIter);

    auto otherGroup = _group;

    // breadth-first search to find what's connected on this side
    Control *writeControl = nullptr;
    bool isExposed = false;
    std::unordered_set<Control*> visitedControls;
    std::queue<Control*> visitQueue;
    std::vector<Control*> orderedControls;

    visitedControls.emplace(this);
    visitQueue.push(this);
    orderedControls.push_back(this);
    while (!visitQueue.empty()) {
        auto visitControl = visitQueue.front();
        visitQueue.pop();

        if (!writeControl && visitControl->direction() == MaximCommon::ControlDirection::OUT) writeControl = visitControl;
        if (visitControl->exposer()) isExposed = true;

        for (const auto &connectedControl : visitControl->connections()) {
            auto ptr = visitedControls.find(connectedControl);
            if (ptr != visitedControls.end()) continue;
            visitedControls.emplace(connectedControl);
            visitQueue.push(connectedControl);
            orderedControls.push_back(connectedControl);

            // remove control from the current group (which will be the other group)
            auto currentGroupIter = std::find(otherGroup->controls().begin(), otherGroup->controls().end(), connectedControl);
            if (currentGroupIter != otherGroup->controls().end()) otherGroup->controls().erase(currentGroupIter);
        }
    }

    // create the new group for this control and connected things
    auto myGroup = std::make_shared<ControlGroup>(_node->surface(), _type, writeControl, orderedControls, isExposed);
    for (const auto &myControl : orderedControls) {
        myControl->setGroup(myGroup);
    }

    // update writer and exposed flag for other group
    otherGroup->setWriter(nullptr);
    otherGroup->setExposed(false);
    for (const auto &otherControl : otherGroup->controls()) {
        if (!otherGroup->writer() && otherControl->direction() == MaximCommon::ControlDirection::OUT) {
            otherGroup->setWriter(otherControl);
        }
        otherGroup->setExposed(otherGroup->isExposed() || otherControl->exposer());

        if (otherGroup->writer() && otherGroup->isExposed()) break;
    }

    _node->surface()->markAsDirty();
}

void Control::cleanup() {
    for (const auto &control : _connections) {
        disconnectFrom(control);
    }
}
