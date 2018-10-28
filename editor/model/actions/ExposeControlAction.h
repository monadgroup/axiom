#pragma once

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class CompositeAction;

    class ExposeControlAction : public Action {
    public:
        ExposeControlAction(const QUuid &controlUuid, const QUuid &exposeUuid, QPoint pos, QSize size, ModelRoot *root);

        static std::unique_ptr<ExposeControlAction> create(const QUuid &controlUuid, const QUuid &exposeUuid,
                                                           QPoint pos, QSize size, ModelRoot *root);

        static std::unique_ptr<ExposeControlAction> create(const QUuid &controlUuid, QPoint pos, QSize size,
                                                           ModelRoot *root);

        static std::unique_ptr<CompositeAction> create(const QUuid &controlUuid, ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &controlUuid() const { return _controlUuid; }

        const QUuid &exposeUuid() const { return _exposeUuid; }

        QPoint pos() const { return _pos; }

        QSize size() const { return _size; }

    private:
        QUuid _controlUuid;
        QUuid _exposeUuid;
        QPoint _pos;
        QSize _size;
    };
}
