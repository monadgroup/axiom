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

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &controlUuid() const { return _controlUuid; }

        const QUuid &exposeUuid() const { return _exposeUuid; }

    private:
        QUuid _controlUuid;
        QUuid _exposeUuid;
    };
}
