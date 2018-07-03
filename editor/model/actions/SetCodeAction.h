#pragma once

#include <QtCore/QUuid>
#include <QtCore/QString>

#include "Action.h"

namespace AxiomModel {

    class SetCodeAction : public Action {
    public:
        SetCodeAction(const QUuid &uuid, QString oldCode, QString newCode, ModelRoot *root);

        static std::unique_ptr<SetCodeAction>
        create(const QUuid &uuid, QString oldCode, QString newCode, ModelRoot *root);

        static std::unique_ptr<SetCodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        bool forward(bool first) override;

        bool backward() override;

    private:

        QUuid uuid;
        QString oldCode;
        QString newCode;
    };

}
