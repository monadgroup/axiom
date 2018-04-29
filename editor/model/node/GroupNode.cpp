#include <iostream>
#include "GroupNode.h"

#include "compiler/runtime/Runtime.h"
#include "../control/NodeControl.h"
#include "../Project.h"
#include "../../util.h"

using namespace AxiomModel;

GroupNode::GroupNode(Schematic *parent, QString name, QPoint pos, QSize size)
    : Node(parent, std::move(name), Type::GROUP, pos, size),
      schematic(std::make_unique<GroupSchematic>(this)) {
    connect(this, &GroupNode::removed,
            this, &GroupNode::onRemoved);
}

void GroupNode::attachRuntime(MaximRuntime::GroupNode *runtime) {
    std::cout << "[GroupNode] Attaching runtime!" << std::endl;
    _runtime = runtime;

    connect(_runtime, &MaximRuntime::GroupNode::extractedChanged,
            this, &GroupNode::extractedChanged);

    // todo: maybe this should go in surface?
    connect(this, &GroupNode::removed,
            this, [this]() {
                _runtime->remove();
            });

    schematic->attachRuntime(runtime->subsurface());

    // todo: add any controls that already exist from the runtime?
}

void GroupNode::createAndAttachRuntime(MaximRuntime::Surface *surface) {
    auto runtime = std::make_unique<MaximRuntime::GroupNode>(surface);
    attachRuntime(runtime.get());
    surface->addNode(std::move(runtime));
}

std::unique_ptr<GridItem> GroupNode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    /*auto schematicParent = dynamic_cast<Schematic *>(newParent);
    assert(schematicParent != nullptr);

    auto moduleNode = std::make_unique<GroupNode>(schematicParent, name(), pos(), size());
    surface.cloneTo(&moduleNode->surface);
    schematic->cloneTo(moduleNode->schematic.get());
    return std::move(moduleNode);*/
    unreachable;
}

void GroupNode::saveValue() {
    Node::saveValue();
    schematic->saveValue();
}

void GroupNode::restoreValue() {
    Node::restoreValue();
    schematic->restoreValue();
}

void GroupNode::serialize(QDataStream &stream, QPoint offset) const {
    Node::serialize(stream, offset);

    stream << (uint32_t) surface.items().size();
    for (const auto &item : surface.items()) {
        auto control = dynamic_cast<NodeControl *>(item.get());
        assert(control);

        stream << control->name();
        stream << (uint8_t) control->type();
        control->serialize(stream, QPoint(0, 0));
    }

    schematic->serialize(stream);
}

void GroupNode::deserialize(QDataStream &stream, QPoint offset) {
    Node::deserialize(stream, offset);

    uint32_t controlCount; stream >> controlCount;
    for (uint32_t i = 0; i < controlCount; i++) {
        QString controlName; stream >> controlName;
        uint8_t intControlType; stream >> intControlType;

        auto control = NodeControl::create(this, (MaximCommon::ControlType) intControlType, controlName);
        control->deserialize(stream, QPoint(0, 0));
        surface.addItem(std::move(control));
    }

    schematic->deserialize(stream);
}

void GroupNode::exposeControl(AxiomModel::NodeControl *control) {
    if (!runtime() || !control->runtime()) return;

    auto newRuntime = runtime()->forwardControl(control->runtime());

    if (control->exposer()) {
        // if the control already has a exposer control in mind, just link to it
        std::cout << "[GroupNode] exposeControl: already has exposer, attaching runtime" << std::endl;
        control->exposer()->attachRuntime(newRuntime);
    } else {
        // otherwise, create a new one
        std::cout << "[GroupNode] exposeControl: doesn't have exposer, creating one" << std::endl;
        auto newControl = addFromRuntime(newRuntime);
        control->setExposer(newControl);
    }

    qt_noop();
}

void GroupNode::onRemoved() {
    std::cout << "Removing GroupNode" << std::endl;
    DO_SUPPRESS(parentSchematic->project()->history, {
        emit schematic->removed();
    });
    std::cout << "Finished removing GroupNode" << std::endl;
}
