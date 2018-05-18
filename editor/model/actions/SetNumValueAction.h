#pragma once

#include <QtCore/QUuid>

#include "Action.h"
#include "compiler/runtime/ValueOperator.h"

namespace AxiomModel {

    class SetNumValueAction : public Action {
    public:
        SetNumValueAction(const QUuid &uuid, MaximRuntime::NumValue beforeVal, MaximRuntime::NumValue afterVal, ModelRoot *root);

        static std::unique_ptr<SetNumValueAction> create(const QUuid &uuid, MaximRuntime::NumValue beforeVal, MaximRuntime::NumValue afterVal, ModelRoot *root);

        static std::unique_ptr<SetNumValueAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        bool forward(bool first) override;

        bool backward() override;

    private:

        QUuid uuid;
        MaximRuntime::NumValue beforeVal;
        MaximRuntime::NumValue afterVal;
    };

}
