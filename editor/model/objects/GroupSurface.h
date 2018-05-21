#pragma once

#include "NodeSurface.h"

namespace AxiomModel {

    class GroupNode;

    class GroupSurface : public NodeSurface {
    public:
        GroupSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        static std::unique_ptr<GroupSurface>
        create(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        QString name() override;

        bool canExposeControl() const override { return true; }

        GroupNode *node() const { return _node; }

    private:
        GroupNode *_node;
    };

}
