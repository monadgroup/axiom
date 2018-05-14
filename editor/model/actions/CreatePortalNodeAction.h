#pragma once

#include "Action.h"
#include "../ConnectionWire.h"
#include "../objects/PortalControl.h"

namespace AxiomModel {

    class CreatePortalNodeAction : public Action {
    public:
        CreatePortalNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name, const QUuid &controlsUuid, ConnectionWire::WireType wireType, PortalControl::PortalType portalType, const QUuid &controlUuid, ModelRoot *root);

        static std::unique_ptr<CreatePortalNodeAction> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name, const QUuid &controlsUuid, ConnectionWire::WireType wireType, PortalControl::PortalType portalType, const QUuid &controlUuid, ModelRoot *root);

        static std::unique_ptr<CreatePortalNodeAction> create(const QUuid &parentUuid, QPoint pos, QString name, ConnectionWire::WireType wireType, PortalControl::PortalType portalType, ModelRoot *root);

        static std::unique_ptr<CreatePortalNodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward() const override;

        void backward() const override;

    private:
        QUuid uuid;
        QUuid parentUuid;
        QPoint pos;
        QString name;
        QUuid controlsUuid;
        ConnectionWire::WireType wireType;
        PortalControl::PortalType portalType;
        QUuid controlUuid;
    };

}
