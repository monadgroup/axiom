#pragma once

#include <QtCore/QUuid>

#include "../objects/Control.h"
#include "Action.h"

namespace AxiomModel {

    class CreateControlAction : public Action {
    public:
        CreateControlAction(const QUuid &uuid, const QUuid &parentUuid, Control::ControlType type, QString name,
                            ModelRoot *root);

        static std::unique_ptr<CreateControlAction> create(const QUuid &uuid, const QUuid &parentUuid,
                                                           Control::ControlType type, QString name, ModelRoot *root);

        static std::unique_ptr<CreateControlAction> create(const QUuid &parentUuid, Control::ControlType type,
                                                           QString name, ModelRoot *root);

        static std::unique_ptr<CreateControlAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, MaximCompiler::Transaction *transaction) override;

        void backward(MaximCompiler::Transaction *transaction) override;

        const QUuid &getUuid() const { return uuid; }

    private:
        QUuid uuid;
        QUuid parentUuid;
        Control::ControlType type;
        QString name;
    };
}
