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
            schematic.get(), &GroupSchematic::removed);
}

void GroupNode::attachRuntime(MaximRuntime::GroupNode *runtime) {
    _runtime = runtime;

    connect(_runtime, &MaximRuntime::GroupNode::controlAdded,
            this, &GroupNode::controlAdded);
    connect(_runtime, &MaximRuntime::GroupNode::extractedChanged,
            this, &GroupNode::extractedChanged);

    // todo: maybe this should go in surface?
    connect(this, &GroupNode::removed,
            [this]() {
                _runtime->remove();
            });

    schematic->attachRuntime(runtime->subsurface());

    // add any controls that might already exist
    for (const std::unique_ptr<MaximRuntime::Control> &control : *runtime) {
        controlAdded(control.get());
    }
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

void GroupNode::serialize(QDataStream &stream) const {
    Node::serialize(stream);
    schematic->serialize(stream);

    // todo: serialize controls
}

void GroupNode::deserialize(QDataStream &stream) {
    Node::deserialize(stream);
    schematic->deserialize(stream);

    // todo: deserialize controls
}

void GroupNode::controlAdded(MaximRuntime::Control *control) {
    // todo: set newControl's exposeBase to the Model Control this SoftControl is for
    // (this info is currently only needed on serialize, so there might be a better way to do it -- maybe the model
    // should be in charge of creating forward controls instead of the compiler?)
    // --> the same might go for other controls, when we need to deserialize
    addFromRuntime(control);
}
