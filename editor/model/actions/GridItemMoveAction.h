#pragma once

#include <QtCore/QUuid>
#include <QtCore/QPoint>

#include "Action.h"

namespace AxiomModel {

    class GridItemMoveAction : public Action {
    public:
        GridItemMoveAction(const QUuid &uuid, QPoint beforePos, QPoint afterPos, AxiomModel::ModelRoot *root);

        static std::unique_ptr<GridItemMoveAction>
        create(const QUuid &uuid, QPoint beforePos, QPoint afterPos, AxiomModel::ModelRoot *root);

        static std::unique_ptr<GridItemMoveAction> deserialize(QDataStream &stream, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first) override;

        void backward() override;

    private:

        QUuid uuid;
        QPoint beforePos;
        QPoint afterPos;
    };

}
