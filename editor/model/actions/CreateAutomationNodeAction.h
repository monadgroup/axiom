#pragma once

#include "../ConnectionWire.h"
#include "../objects/PortalControl.h"
#include "Action.h"

namespace AxiomModel {

    class CreateAutomationNodeAction : public Action {
    public:
        CreateAutomationNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                   const QUuid &controlsUuid, const QUuid &controlUuid, ModelRoot *root);

        static std::unique_ptr<CreateAutomationNodeAction> create(const QUuid &uuid, const QUuid &parentUuid,
                                                                  QPoint pos, QString name, const QUuid &controlsUuid,
                                                                  const QUuid &controlUuid, ModelRoot *root);

        static std::unique_ptr<CreateAutomationNodeAction> create(const QUuid &parentUuid, QPoint pos, QString name,
                                                                  ModelRoot *root);

        static std::unique_ptr<CreateAutomationNodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, MaximCompiler::Transaction *transaction) override;

        void backward(MaximCompiler::Transaction *transaction) override;

    private:
        QUuid uuid;
        QUuid parentUuid;
        QPoint pos;
        QString name;
        QUuid controlsUuid;
        QUuid controlUuid;
    };
}
