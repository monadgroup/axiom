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

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &getUuid() const { return uuid; }

    private:
        QUuid uuid;
        QUuid parentUuid;
        Control::ControlType type;
        QString name;
    };
}
