#pragma once

#include "Action.h"
#include "../ConnectionWire.h"
#include "../objects/PortalControl.h"

namespace AxiomModel {

    class CreateAutomationNodeAction : public Action {
    public:
        CreateAutomationNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                   const QUuid &controlsUuid, const QUuid &controlUuid, ModelRoot *root);

        static std::unique_ptr<CreateAutomationNodeAction>
        create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name, const QUuid &controlsUuid,
               const QUuid &controlUuid, ModelRoot *root);

        static std::unique_ptr<CreateAutomationNodeAction>
        create(const QUuid &parentUuid, QPoint pos, QString name, ModelRoot *root);

        static std::unique_ptr<CreateAutomationNodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        bool forward(bool first) override;

        bool backward() override;

    private:
        QUuid uuid;
        QUuid parentUuid;
        QPoint pos;
        QString name;
        QUuid controlsUuid;
        QUuid controlUuid;
    };

}
