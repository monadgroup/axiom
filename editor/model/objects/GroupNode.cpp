#include "GroupNode.h"

#include "Control.h"
#include "ControlSurface.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "compiler/runtime/Surface.h"
#include "compiler/runtime/GroupNode.h"

using namespace AxiomModel;

GroupNode::GroupNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                     const QUuid &controlsUuid, const QUuid &innerUuid, AxiomModel::ModelRoot *root)
    : Node(NodeType::GROUP_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root),
      _nodes(findLater<NodeSurface *>(root->nodeSurfaces(), innerUuid)) {
}

std::unique_ptr<GroupNode> GroupNode::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                             bool selected, QString name, const QUuid &controlsUuid,
                                             const QUuid &innerUuid, AxiomModel::ModelRoot *root) {
    return std::make_unique<GroupNode>(uuid, parentUuid, pos, size, selected, name, controlsUuid, innerUuid, root);
}

std::unique_ptr<GroupNode> GroupNode::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                  QPoint pos, QSize size, bool selected, QString name,
                                                  const QUuid &controlsUuid, AxiomModel::ModelRoot *root) {
    QUuid innerUuid;
    stream >> innerUuid;

    return create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, innerUuid, root);
}

void GroupNode::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Node::serialize(stream, parent, withContext);
    stream << (*_nodes.value())->uuid();
}

void GroupNode::createAndAttachRuntime(MaximRuntime::Surface *parent) {
    auto runtime = std::make_unique<MaximRuntime::GroupNode>(parent);
    attachRuntime(runtime.get());
    parent->addNode(std::move(runtime));
}

void GroupNode::attachRuntime(MaximRuntime::GroupNode *runtime) {
    assert(!_runtime);
    _runtime = runtime;

    runtime->extractedChanged.connect(this, &GroupNode::setExtracted);

    controls().then([this](ControlSurface *const &controls) {
        controls->controls().itemAdded.connect(this, &GroupNode::surfaceControlAdded);
        for (const auto &control : controls->controls()) {
            surfaceControlAdded(control);
        }
    });

    removed.connect(this, &GroupNode::detachRuntime);

    _nodes.then([runtime](NodeSurface *const &nodes) {
        nodes->attachRuntime(runtime->subsurface());
    });
}

void GroupNode::detachRuntime() {
    if (_runtime) (*_runtime)->remove();
    _runtime.reset();
}

void GroupNode::saveValue() {
    if (_nodes.value()) (*_nodes.value())->saveValue();
    Node::saveValue();
}

void GroupNode::restoreValue() {
    if (_nodes.value()) (*_nodes.value())->restoreValue();
    Node::restoreValue();
}

void GroupNode::remove() {
    if (_nodes.value()) (*_nodes.value())->remove();
    Node::remove();
}

void GroupNode::surfaceControlAdded(AxiomModel::Control *control) {
    if (!_runtime) return;

    // find a runtime control to attach
    for (const std::unique_ptr<MaximRuntime::Control> &runtimeControl : **_runtime) {
        if (!control->canAttachRuntime(runtimeControl.get())) continue;

        control->attachRuntime(runtimeControl.get());
        return;
    }
}
