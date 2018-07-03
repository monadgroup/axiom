#pragma once

#include <QtCore/QUuid>

#include "Action.h"
#include "../objects/Control.h"

namespace AxiomModel {

    class CreateControlAction : public Action {
    public:
        CreateControlAction(const QUuid &uuid, const QUuid &parentUuid, Control::ControlType type, QString name,
                            ModelRoot *root);

        static std::unique_ptr<CreateControlAction>
        create(const QUuid &uuid, const QUuid &parentUuid, Control::ControlType type, QString name, ModelRoot *root);

        static std::unique_ptr<CreateControlAction>
        create(const QUuid &parentUuid, Control::ControlType type, QString name, ModelRoot *root);

        static std::unique_ptr<CreateControlAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        bool forward(bool first) override;

        bool backward() override;

    private:
        QUuid uuid;
        QUuid parentUuid;
        Control::ControlType type;
        QString name;
    };

}
