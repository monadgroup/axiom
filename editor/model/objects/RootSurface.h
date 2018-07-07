#pragma once

#include <vector>

#include "../ConnectionWire.h"
#include "NodeSurface.h"
#include "PortalControl.h"

namespace AxiomModel {

    struct RootSurfacePortal {
        PortalControl::PortalType portalType;
        ConnectionWire::WireType valueType;
        QString name;

        RootSurfacePortal(PortalControl::PortalType portalType, ConnectionWire::WireType valueType, QString name)
            : portalType(portalType), valueType(valueType), name(std::move(name)) {}
    };

    struct RootSurfaceCompileMeta {
        std::vector<RootSurfacePortal> portals;

        explicit RootSurfaceCompileMeta(std::vector<RootSurfacePortal> portals) : portals(std::move(portals)) {}
    };

    class RootSurface : public NodeSurface {
    public:
        RootSurface(const QUuid &uuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        QString name() override { return "Root"; }

        bool canExposeControl() const override { return false; }

        bool canHaveAutomation() const override { return true; }

        uint64_t getRuntimeId() override { return 0; }

        const std::optional<RootSurfaceCompileMeta> &compileMeta() const { return _compileMeta; }

        void setCompileMeta(std::optional<RootSurfaceCompileMeta> compileMeta) {
            _compileMeta = std::move(compileMeta);
        }

    private:
        std::optional<RootSurfaceCompileMeta> _compileMeta;
    };
}
