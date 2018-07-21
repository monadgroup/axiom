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

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &uuid() const { return _uuid; }

        const QPoint &beforePos() const { return _beforePos; }

        const QPoint &afterPos() const { return _afterPos; }

    private:
        QUuid _uuid;
        QPoint _beforePos;
        QPoint _afterPos;
    };
}
