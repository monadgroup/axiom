#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class ModelRoot;
    class Node;
    class CustomNode;
    class GroupNode;
    class PortalNode;
    class ReferenceMapper;

    namespace NodeSerializer {
        void serialize(Node *node, QDataStream &stream);

        std::unique_ptr<Node> deserialize(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                          const QUuid &parentUuid, ReferenceMapper *ref, ModelRoot *root);

        void serializeCustom(CustomNode *node, QDataStream &stream);

        std::unique_ptr<CustomNode> deserializeCustom(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                      const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                                                      QString name, const QUuid &controlsUuid, ReferenceMapper *ref,
                                                      ModelRoot *root);

        void serializeGroup(GroupNode *node, QDataStream &stream);

        std::unique_ptr<GroupNode> deserializeGroup(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                    const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                                                    QString name, const QUuid &controlsUuid, ReferenceMapper *ref,
                                                    ModelRoot *root);

        void serializePortal(PortalNode *node, QDataStream &stream);

        std::unique_ptr<PortalNode> deserializePortal(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                      const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                                                      QString name, const QUuid &controlsUuid, ReferenceMapper *ref,
                                                      ModelRoot *root);
    }
}
