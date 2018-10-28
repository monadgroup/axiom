#pragma once

#include "../ConnectionWire.h"
#include "../objects/PortalControl.h"
#include "Action.h"

namespace AxiomModel {

    class CreatePortalNodeAction : public Action {
    public:
        CreatePortalNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                               const QUuid &controlsUuid, ConnectionWire::WireType wireType,
                               PortalControl::PortalType portalType, uint64_t portalId, const QUuid &controlUuid,
                               ModelRoot *root);

        static std::unique_ptr<CreatePortalNodeAction> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                              QString name, const QUuid &controlsUuid,
                                                              ConnectionWire::WireType wireType,
                                                              PortalControl::PortalType portalType, uint64_t portalId,
                                                              const QUuid &controlUuid, ModelRoot *root);

        static std::unique_ptr<CreatePortalNodeAction> create(const QUuid &parentUuid, QPoint pos, QString name,
                                                              ConnectionWire::WireType wireType,
                                                              PortalControl::PortalType portalType, ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const QUuid &parentUuid() const { return _parentUuid; }

        const QPoint &pos() const { return _pos; }

        const QString &name() const { return _name; }

        const QUuid &controlsUuid() const { return _controlsUuid; }

        const ConnectionWire::WireType &wireType() const { return _wireType; }

        const PortalControl::PortalType &portalType() const { return _portalType; }

        uint64_t portalId() const { return _portalId; }

        const QUuid &controlUuid() const { return _controlUuid; }

    private:
        QUuid _uuid;
        QUuid _parentUuid;
        QPoint _pos;
        QString _name;
        QUuid _controlsUuid;
        ConnectionWire::WireType _wireType;
        PortalControl::PortalType _portalType;
        uint64_t _portalId;
        QUuid _controlUuid;
    };
}
