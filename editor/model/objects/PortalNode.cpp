#include "PortalNode.h"

#include "ControlSurface.h"
#include "../WatchSequenceOperators.h"
#include "compiler/runtime/IONode.h"
#include "compiler/runtime/RootSurface.h"

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

void PortalNode::createAndAttachRuntime(MaximRuntime::Surface *parent) {
    controls().then([this, parent](ControlSurface *controls) {
        takeAtLater(controls->controls(), 0).then([this, parent](Control *control) {
            auto portalControl = dynamic_cast<PortalControl*>(control);
            auto rootParent = dynamic_cast<MaximRuntime::RootSurface *>(parent);
            if (portalControl->portalType() == PortalControl::PortalType::AUTOMATION) {
                attachRuntime(rootParent->addAutomationNode());
            }
        });
    });
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
