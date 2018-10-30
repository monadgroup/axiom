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

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const QUuid &parentUuid() const { return _parentUuid; }

        const QUuid &controlA() const { return _controlA; }

        const QUuid &controlB() const { return _controlB; }

    private:
        QUuid _uuid;
        QUuid _parentUuid;
        QUuid _controlA;
        QUuid _controlB;
    };
}
