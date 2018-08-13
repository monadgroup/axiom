#pragma once

#include <vector>

#include "../ConnectionWire.h"
#include "NodeSurface.h"
#include "PortalControl.h"

namespace AxiomModel {

    struct RootSurfacePortal {
        uint64_t id;
        PortalControl::PortalType portalType;
        ConnectionWire::WireType valueType;
        QString name;

        RootSurfacePortal(uint64_t id, PortalControl::PortalType portalType, ConnectionWire::WireType valueType,
                          QString name)
            : id(id), portalType(portalType), valueType(valueType), name(std::move(name)) {}
    };

    struct RootSurfaceCompileMeta {
        std::vector<RootSurfacePortal> portals;

        explicit RootSurfaceCompileMeta(std::vector<RootSurfacePortal> portals) : portals(std::move(portals)) {}
    };

    class RootSurface : public NodeSurface {
    public:
        RootSurface(const QUuid &uuid, QPointF pan, float zoom, size_t nextPortalId, AxiomModel::ModelRoot *root);

        QString name() override { return "Root"; }

        bool canExposeControl() const override { return false; }

        bool canHavePortals() const override { return true; }

        uint64_t getRuntimeId() override { return 0; }

        const std::optional<RootSurfaceCompileMeta> &compileMeta() const { return _compileMeta; }

        void setCompileMeta(std::optional<RootSurfaceCompileMeta> compileMeta) {
            _compileMeta = std::move(compileMeta);
        }

        uint64_t nextPortalId() const { return _nextPortalId; }

        uint64_t takePortalId() { return _nextPortalId++; }

    private:
        std::optional<RootSurfaceCompileMeta> _compileMeta;
        uint64_t _nextPortalId;
    };
}
