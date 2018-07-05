#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class ExposeControlAction : public Action {
    public:
        ExposeControlAction(const QUuid &controlUuid, const QUuid &exposeUuid, ModelRoot *root);

        static std::unique_ptr<ExposeControlAction> create(const QUuid &controlUuid, const QUuid &exposeUuid,
                                                           ModelRoot *root);

        static std::unique_ptr<ExposeControlAction> create(const QUuid &controlUuid, ModelRoot *root);

        static std::unique_ptr<ExposeControlAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, MaximCompiler::Transaction *transaction) override;

        void backward(MaximCompiler::Transaction *transaction) override;

    private:
        QUuid controlUuid;
        QUuid exposeUuid;
    };
}
