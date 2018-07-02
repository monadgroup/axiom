#include "AutomationNode.h"

#include "ControlSurface.h"
#include "Control.h"
#include "../WatchSequenceOperators.h"

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
