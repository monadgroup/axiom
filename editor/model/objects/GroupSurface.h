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

        bool canHaveAutomation() const override { return false; }

        GroupNode *node() const { return _node; }

        uint64_t getRuntimeId(MaximCompiler::Runtime &runtime) override;

    private:
        GroupNode *_node;
        uint64_t runtimeId = 0;
    };

}
