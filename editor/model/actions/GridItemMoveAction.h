#pragma once

#include <QtCore/QPoint>
#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class GridItemMoveAction : public Action {
    public:
        GridItemMoveAction(const QUuid &uuid, QPoint beforePos, QPoint afterPos, AxiomModel::ModelRoot *root);

        static std::unique_ptr<GridItemMoveAction> create(const QUuid &uuid, QPoint beforePos, QPoint afterPos,
                                                          AxiomModel::ModelRoot *root);

        static std::unique_ptr<GridItemMoveAction> deserialize(QDataStream &stream, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

    private:
        QUuid uuid;
        QPoint beforePos;
        QPoint afterPos;
    };
}
