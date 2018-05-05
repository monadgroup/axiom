#pragma once

#include "Node.h"
#include "NodeSurface.h"

namespace AxiomModel {

    class GroupNode : public Node {
    public:
        GroupNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, const QUuid &innerUuid, ModelRoot *root);

        static std::unique_ptr<GroupNode> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        Promise<NodeSurface*> &nodes() { return _nodes; }

        const Promise<NodeSurface*> &nodes() const { return _nodes; }

    private:
        Promise<NodeSurface*> _nodes;
    };

}
