#pragma once

#include <QtCore/QRect>
#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class GridItemSizeAction : public Action {
    public:
        GridItemSizeAction(const QUuid &uuid, QRect beforeRect, QRect afterRect, ModelRoot *root);

        static std::unique_ptr<GridItemSizeAction> create(const QUuid &uuid, QRect beforeRect, QRect afterRect,
                                                          ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const QRect &beforeRect() const { return _beforeRect; }

        const QRect &afterRect() const { return _afterRect; }

    private:
        QUuid _uuid;
        QRect _beforeRect;
        QRect _afterRect;
    };
}
