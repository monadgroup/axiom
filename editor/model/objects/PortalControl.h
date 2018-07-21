#pragma once

#include "Control.h"

namespace AxiomModel {

    class PortalControl : public Control {
    public:
        enum class PortalType { INPUT, OUTPUT, AUTOMATION };

        PortalControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                      bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                      ConnectionWire::WireType wireType, PortalType portalType, ModelRoot *root);

        static std::unique_ptr<PortalControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                     bool selected, QString name, bool showName,
                                                     const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                     ConnectionWire::WireType wireType, PortalType portalType,
                                                     ModelRoot *root);

        bool isMovable() const override { return false; }

        PortalType portalType() const { return _portalType; }

        void doRuntimeUpdate() override {}

    private:
        PortalType _portalType;
    };
}
