#include "PortalNode.h"

#include "ControlSurface.h"
#include "Control.h"
#include "../WatchSequenceOperators.h"
#include "compiler/runtime/IONode.h"

using namespace AxiomModel;

PortalNode::PortalNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       const QUuid &controlsUuid, AxiomModel::ModelRoot *root)
    : Node(NodeType::PORTAL_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root) {
}

std::unique_ptr<PortalNode> PortalNode::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, const QUuid &controlsUuid,
                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<PortalNode>(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root);
}

std::unique_ptr<PortalNode> PortalNode::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    QPoint pos, QSize size, bool selected, QString name,
                                                    const QUuid &controlsUuid, ReferenceMapper *ref,
                                                    AxiomModel::ModelRoot *root) {
    return create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root);
}

void PortalNode::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Node::serialize(stream, parent, withContext);
}

void PortalNode::attachRuntime(MaximRuntime::IONode *runtime) {
    runtime->setName(name().toStdString());
    controls().then([runtime](ControlSurface *controls) {
        auto controlRuntime = runtime->control();
        takeAtLater(controls->controls(), 0).then([controlRuntime](Control *control) {
            control->attachRuntime(controlRuntime);
        });
    });
}
