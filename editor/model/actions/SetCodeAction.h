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

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const QString &oldCode() const { return _oldCode; }

        const QString &newCode() const { return _newCode; }

        const std::vector<std::unique_ptr<Action>> &controlActions() const { return _controlActions; }

        std::vector<std::unique_ptr<Action>> &controlActions() { return _controlActions; }

    private:
        QUuid _uuid;
        QString _oldCode;
        QString _newCode;
        std::vector<std::unique_ptr<Action>> _controlActions;
    };
}
