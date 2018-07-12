#pragma once

#include <QtCore/QUuid>

#include "../Value.h"
#include "Action.h"

namespace AxiomModel {

    class SetNumValueAction : public Action {
    public:
        SetNumValueAction(const QUuid &uuid, NumValue beforeVal, NumValue afterVal, ModelRoot *root);

        static std::unique_ptr<SetNumValueAction> create(const QUuid &uuid, NumValue beforeVal, NumValue afterVal,
                                                         ModelRoot *root);

        static std::unique_ptr<SetNumValueAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

    private:
        QUuid uuid;
        NumValue beforeVal;
        NumValue afterVal;
    };
}
