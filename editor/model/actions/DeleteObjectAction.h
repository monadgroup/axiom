#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class DeleteObjectAction : public Action {
    public:
        DeleteObjectAction(const QUuid &uuid, const QUuid &parentUuid, QByteArray buffer, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> create(const QUuid &uuid, const QUuid &parentUuid, QByteArray buffer, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> create(const QUuid &uuid, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> deserialize(QDataStream &stream, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward() const override;

        void backward() const override;

    private:

        QUuid uuid;
        QUuid parentUuid;
        QByteArray buffer;
    };

}
