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
    pos = ref->mapPos(parentUuid, pos);

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
    stream << node->panelSize();
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

    // Before schema version 7 (Axiom version 0.5.0) only the panel height could be changed, so the width was not
    // stored. We use a width of 3, which is the default for new nodes.
    // Additionally, in schema version 7, the units of the width and height changed to be in surface grid coords
    // instead of pixel coords. Grids were 50px in size, so this is fixed by dividing by 50.
    QSizeF panelSize;
    if (version >= 7) {
        stream >> panelSize;
    } else {
        float panelPixelHeight;
        stream >> panelPixelHeight;

        panelSize = QSizeF(3, panelPixelHeight / 50);
    }

    return CustomNode::create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, std::move(code),
                              isPanelOpen, panelSize, root);
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
