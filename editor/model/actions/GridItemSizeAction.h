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

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &uuid() const { return _uuid; }

        const QRect &beforeRect() const { return _beforeRect; }

        const QRect &afterRect() const { return _afterRect; }

    private:
        QUuid _uuid;
        QRect _beforeRect;
        QRect _afterRect;
    };
}
