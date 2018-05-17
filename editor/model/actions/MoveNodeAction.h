#pragma once

#include <QtCore/QUuid>
#include <QtCore/QPoint>

#include "Action.h"

namespace AxiomModel {

    class MoveNodeAction : public Action {
    public:
        MoveNodeAction(const QUuid &uuid, QPoint beforePos, QPoint afterPos, AxiomModel::ModelRoot *root);

        static std::unique_ptr<MoveNodeAction> create(const QUuid &uuid, QPoint beforePos, QPoint afterPos, AxiomModel::ModelRoot *root);

        static std::unique_ptr<MoveNodeAction> deserialize(QDataStream &stream, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first) override;

        void backward() override;

    private:

        QUuid uuid;
        QPoint beforePos;
        QPoint afterPos;
    };

}
