#include "GroupNode.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

GroupNode::GroupNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                     const QUuid &controlsUuid, const QUuid &innerUuid, AxiomModel::ModelRoot *root)
    : Node(NodeType::GROUP_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root),
      _nodes(findLater<NodeSurface*>(root->nodeSurfaces(), innerUuid)) {
}

std::unique_ptr<GroupNode> GroupNode::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                  QPoint pos, QSize size, bool selected, QString name,
                                                  const QUuid &controlsUuid, AxiomModel::ModelRoot *root) {
    QUuid innerUuid; stream >> innerUuid;

    return std::make_unique<GroupNode>(uuid, parentUuid, pos, size, selected, name, controlsUuid, innerUuid, root);
}

void GroupNode::serialize(QDataStream &stream) const {
    Node::serialize(stream);
    stream << (*_nodes.value())->uuid();
}
