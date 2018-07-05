#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetShowNameAction : public Action {
    public:
        SetShowNameAction(const QUuid &uuid, bool beforeVal, bool afterVal, ModelRoot *root);

        static std::unique_ptr<SetShowNameAction> create(const QUuid &uuid, bool beforeVal, bool afterVal,
                                                         ModelRoot *root);

        static std::unique_ptr<SetShowNameAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, MaximCompiler::Transaction *transaction) override;

        void backward(MaximCompiler::Transaction *transaction) override;

    private:
        QUuid uuid;
        bool beforeVal;
        bool afterVal;
    };
}
