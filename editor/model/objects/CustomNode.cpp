#include "CustomNode.h"

using namespace AxiomModel;

CustomNode::CustomNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       const QUuid &controlsUuid, QString code, AxiomModel::ModelRoot *root)
    : Node(NodeType::CUSTOM_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root),
      _code(std::move(code)) {
}

std::unique_ptr<CustomNode> CustomNode::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    QPoint pos, QSize size, bool selected, QString name,
                                                    const QUuid &controlsUuid, AxiomModel::ModelRoot *root) {
    QString code; stream >> code;

    return std::make_unique<CustomNode>(uuid, parentUuid, pos, size, selected, name, controlsUuid, code, root);
}

void CustomNode::serialize(QDataStream &stream) const {
    Node::serialize(stream);
    stream << _code;
}
