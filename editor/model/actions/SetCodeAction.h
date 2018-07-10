#pragma once

#include <QtCore/QString>
#include <QtCore/QUuid>

#include "Action.h"
#include "CompositeAction.h"

namespace AxiomModel {

    class SetCodeAction : public Action {
    public:
        SetCodeAction(const QUuid &uuid, QString oldCode, QString newCode,
                      std::unique_ptr<CompositeAction> controlActions, ModelRoot *root);

        static std::unique_ptr<SetCodeAction> create(const QUuid &uuid, QString oldCode, QString newCode,
                                                     std::unique_ptr<CompositeAction> controlActions, ModelRoot *root);

        static std::unique_ptr<SetCodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        CompositeAction *controlActions() const { return _controlActions.get(); }

    private:
        QUuid uuid;
        QString oldCode;
        QString newCode;
        std::unique_ptr<CompositeAction> _controlActions;
    };
}
