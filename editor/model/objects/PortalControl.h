#pragma once

#include "Control.h"

namespace AxiomModel {

    class PortalControl : public Control {
    public:
        enum class PortalType {
            INPUT,
            OUTPUT,
            AUTOMATION
        };

        PortalControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, ConnectionWire::WireType wireType, PortalType portalType, ModelRoot *root);

        static std::unique_ptr<PortalControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, ConnectionWire::WireType wireType, PortalType portalType, ModelRoot *root);

        static std::unique_ptr<PortalControl> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, ConnectionWire::WireType wireType, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        PortalType portalType() const { return _portalType; }

    private:
        PortalType _portalType;

    };

}
