#include "AutomationNode.h"

#include "ControlSurface.h"
#include "Control.h"
#include "../WatchSequenceOperators.h"
#include "compiler/runtime/IONode.h"
#include "compiler/runtime/RootSurface.h"

using namespace AxiomModel;

AutomationNode::AutomationNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                               QString name, const QUuid &controlsUuid, AxiomModel::ModelRoot *root)
    : Node(NodeType::AUTOMATION_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root) {
}

std::unique_ptr<AutomationNode> AutomationNode::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                       QSize size, bool selected, QString name,
                                                       const QUuid &controlsUuid, AxiomModel::ModelRoot *root) {
    return std::make_unique<AutomationNode>(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root);
}

std::unique_ptr<AutomationNode> AutomationNode::deserialize(QDataStream &stream, const QUuid &uuid,
                                                            const QUuid &parentUuid, QPoint pos, QSize size,
                                                            bool selected, QString name, const QUuid &controlsUuid,
                                                            AxiomModel::ReferenceMapper *ref,
                                                            AxiomModel::ModelRoot *root) {
    return create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root);
}

void AutomationNode::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Node::serialize(stream, parent, withContext);
}

void AutomationNode::createAndAttachRuntime(MaximRuntime::Surface *parent) {
    auto rootParent = dynamic_cast<MaximRuntime::RootSurface *>(parent);
    assert(rootParent);
    attachRuntime(rootParent->addAutomationNode());
}

void AutomationNode::attachRuntime(MaximRuntime::IONode *runtime) {
    assert(!_runtime);
    _runtime = runtime;
    runtime->setName(name().toStdString());
    controls().then([runtime](ControlSurface *controls) {
        auto controlRuntime = runtime->control();
        takeAtLater(controls->controls(), 0).then([controlRuntime](Control *control) {
            control->attachRuntime(controlRuntime);
        });
    });

    removed.connect(this, &AutomationNode::detachRuntime);
}

void AutomationNode::detachRuntime() {
    if (_runtime) (*_runtime)->remove();
    _runtime.reset();
}
