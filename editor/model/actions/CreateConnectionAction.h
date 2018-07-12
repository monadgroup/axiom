#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class CreateConnectionAction : public Action {
    public:
        CreateConnectionAction(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA, const QUuid &controlB,
                               ModelRoot *root);

        static std::unique_ptr<CreateConnectionAction> create(const QUuid &uuid, const QUuid &parentUuid,
                                                              const QUuid &controlA, const QUuid &controlB,
                                                              ModelRoot *root);

        static std::unique_ptr<CreateConnectionAction> create(const QUuid &parentUuid, const QUuid &controlA,
                                                              const QUuid &controlB, ModelRoot *root);

        static std::unique_ptr<CreateConnectionAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

    private:
        QUuid uuid;
        QUuid parentUuid;
        QUuid controlA;
        QUuid controlB;
    };
}
