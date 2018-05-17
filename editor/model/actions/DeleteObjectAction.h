#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class ModelObject;

    class DeleteObjectAction : public Action {
    public:
        DeleteObjectAction(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction>
        create(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> create(const ModelObject *object);

        static std::unique_ptr<DeleteObjectAction> deserialize(QDataStream &stream, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first) override;

        void backward() override;

    private:

        QUuid uuid;
        QByteArray buffer;
    };

}
