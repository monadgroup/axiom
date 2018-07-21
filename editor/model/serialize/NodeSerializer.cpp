#include "NodeSerializer.h"

#include "../../util.h"
#include "../ReferenceMapper.h"
#include "../objects/ControlSurface.h"
#include "../objects/CustomNode.h"
#include "../objects/GroupNode.h"
#include "../objects/Node.h"
#include "../objects/PortalNode.h"

using namespace AxiomModel;

void NodeSerializer::serialize(AxiomModel::Node *node, QDataStream &stream) {
    stream << (uint8_t) node->nodeType();
    stream << node->pos();
    stream << node->size();
    stream << node->isSelected();
    stream << node->name();
    stream << (*node->controls().value())->uuid();

    if (auto custom = dynamic_cast<CustomNode *>(node))
        serializeCustom(custom, stream);
    else if (auto group = dynamic_cast<GroupNode *>(node))
        serializeGroup(group, stream);
    else if (auto portal = dynamic_cast<PortalNode *>(node))
        serializePortal(portal, stream);
    else
        unreachable;
}

std::unique_ptr<Node> NodeSerializer::deserialize(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                  const QUuid &parentUuid, AxiomModel::ReferenceMapper *ref,
                                                  AxiomModel::ModelRoot *root) {
    uint8_t nodeTypeInt;
    stream >> nodeTypeInt;

    QPoint pos;
    stream >> pos;

    QSize size;
    stream >> size;

    bool selected;
    stream >> selected;

    QString name;
    stream >> name;

    QUuid controlsUuid;
    stream >> controlsUuid;
    controlsUuid = ref->mapUuid(controlsUuid);

    switch ((Node::NodeType) nodeTypeInt) {
    case Node::NodeType::CUSTOM_NODE:
        return deserializeCustom(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid,
                                 ref, root);
    case Node::NodeType::GROUP_NODE:
        return deserializeGroup(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid,
                                ref, root);
    case Node::NodeType::PORTAL_NODE:
        return deserializePortal(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid,
                                 ref, root);
    default:
        unreachable;
    }
}

void NodeSerializer::serializeCustom(AxiomModel::CustomNode *node, QDataStream &stream) {
    stream << node->code();
    stream << node->isPanelOpen();
    stream << node->panelHeight();
}

std::unique_ptr<CustomNode> NodeSerializer::deserializeCustom(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                              const QUuid &parentUuid, QPoint pos, QSize size,
                                                              bool selected, QString name, const QUuid &controlsUuid,
                                                              AxiomModel::ReferenceMapper *ref,
                                                              AxiomModel::ModelRoot *root) {
    QString code;
    stream >> code;
    bool isPanelOpen;
    stream >> isPanelOpen;
    float panelHeight;
    stream >> panelHeight;

    return CustomNode::create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, std::move(code),
                              isPanelOpen, panelHeight, root);
}

void NodeSerializer::serializeGroup(AxiomModel::GroupNode *node, QDataStream &stream) {
    stream << (*node->nodes().value())->uuid();
}

std::unique_ptr<GroupNode> NodeSerializer::deserializeGroup(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                            const QUuid &parentUuid, QPoint pos, QSize size,
                                                            bool selected, QString name, const QUuid &controlsUuid,
                                                            AxiomModel::ReferenceMapper *ref,
                                                            AxiomModel::ModelRoot *root) {
    QUuid innerUuid;
    stream >> innerUuid;
    innerUuid = ref->mapUuid(innerUuid);

    return GroupNode::create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, innerUuid, root);
}

void NodeSerializer::serializePortal(AxiomModel::PortalNode *node, QDataStream &stream) {}

std::unique_ptr<PortalNode> NodeSerializer::deserializePortal(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                              const QUuid &parentUuid, QPoint pos, QSize size,
                                                              bool selected, QString name, const QUuid &controlsUuid,
                                                              AxiomModel::ReferenceMapper *ref,
                                                              AxiomModel::ModelRoot *root) {
    return PortalNode::create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root);
}
