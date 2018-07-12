#pragma once

#include <QtCore/QString>
#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetCodeAction : public Action {
    public:
        SetCodeAction(const QUuid &uuid, QString oldCode, QString newCode,
                      std::vector<std::unique_ptr<Action>> controlActions, ModelRoot *root);

        static std::unique_ptr<SetCodeAction> create(const QUuid &uuid, QString oldCode, QString newCode,
                                                     std::vector<std::unique_ptr<Action>> controlActions,
                                                     ModelRoot *root);

        static std::unique_ptr<SetCodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        std::vector<std::unique_ptr<Action>> &controlActions() { return _controlActions; }

    private:
        QUuid uuid;
        QString oldCode;
        QString newCode;
        std::vector<std::unique_ptr<Action>> _controlActions;
    };
}
