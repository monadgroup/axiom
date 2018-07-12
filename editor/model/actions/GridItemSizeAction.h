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

        static std::unique_ptr<GridItemSizeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

    private:
        QUuid uuid;
        QRect beforeRect;
        QRect afterRect;
    };
}
