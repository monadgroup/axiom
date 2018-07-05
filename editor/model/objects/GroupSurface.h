#pragma once

#include "NodeSurface.h"

namespace AxiomModel {

    class GroupNode;

    struct GroupSurfacePortal {
        std::vector<QUuid> externalControls;
        bool valueWritten;
        bool valueRead;
        bool isExtractor;

        GroupSurfacePortal(std::vector<QUuid> externalControls, bool valueWritten, bool valueRead, bool isExtractor)
            : externalControls(std::move(externalControls)), valueWritten(valueWritten), valueRead(valueRead),
              isExtractor(isExtractor) {}
    };

    struct GroupSurfaceCompileMeta {
        std::vector<GroupSurfacePortal> portals;

        explicit GroupSurfaceCompileMeta(std::vector<GroupSurfacePortal> portals) : portals(std::move(portals)) {}
    };

    class GroupSurface : public NodeSurface {
    public:
        GroupSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        static std::unique_ptr<GroupSurface> create(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                                                    AxiomModel::ModelRoot *root);

        QString name() override;

        bool canExposeControl() const override { return true; }

        bool canHaveAutomation() const override { return false; }

        GroupNode *node() const { return _node; }

        uint64_t getRuntimeId() override { return runtimeId; }

        void attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) override;

        const std::optional<GroupSurfaceCompileMeta> &compileMeta() const { return _compileMeta; }

        void setCompileMeta(std::optional<GroupSurfaceCompileMeta> compileMeta) {
            _compileMeta = std::move(compileMeta);
        }

    private:
        GroupNode *_node;
        uint64_t runtimeId = 0;
        std::optional<GroupSurfaceCompileMeta> _compileMeta;
    };
}
