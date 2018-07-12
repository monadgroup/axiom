#pragma once

#include <QtCore/QUuid>

#include "../Sequence.h"
#include "Action.h"

namespace AxiomModel {

    class ModelObject;

    class DeleteObjectAction : public Action {
    public:
        DeleteObjectAction(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> create(const QUuid &uuid, QByteArray buffer,
                                                          AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> create(const QUuid &uuid, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> deserialize(QDataStream &stream, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems);

        void backward(std::vector<QUuid> &compileItems);

    private:
        QUuid uuid;
        QByteArray buffer;

        Sequence<ModelObject *> getLinkedItems(const QUuid &seed) const;

        Sequence<ModelObject *> getRemoveItems() const;
    };
}
