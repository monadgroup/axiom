#pragma once

#include <QtCore/QUuid>
#include <QtCore/QRect>

#include "Action.h"

namespace AxiomModel {

    class GridItemSizeAction : public Action {
    public:
        GridItemSizeAction(const QUuid &uuid, QRect beforeRect, QRect afterRect, ModelRoot *root);

        static std::unique_ptr<GridItemSizeAction>
        create(const QUuid &uuid, QRect beforeRect, QRect afterRect, ModelRoot *root);

        static std::unique_ptr<GridItemSizeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        bool forward(bool first) override;

        bool backward() override;

    private:

        QUuid uuid;
        QRect beforeRect;
        QRect afterRect;
    };

}
