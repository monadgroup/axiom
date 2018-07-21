#include "PortalNode.h"

#include "../WatchSequenceOperators.h"
#include "Control.h"
#include "ControlSurface.h"

using namespace AxiomModel;

PortalNode::PortalNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       const QUuid &controlsUuid, AxiomModel::ModelRoot *root)
    : Node(NodeType::PORTAL_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root) {}

std::unique_ptr<PortalNode> PortalNode::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, const QUuid &controlsUuid,
                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<PortalNode>(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root);
}
