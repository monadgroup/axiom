#pragma once

#include <QtCore/QUuid>

#include "Action.h"
#include "../Sequence.h"

namespace AxiomModel {

    class ModelObject;

    class DeleteObjectAction : public Action {
    public:
        DeleteObjectAction(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction>
        create(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> create(const QUuid &uuid, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> deserialize(QDataStream &stream, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        bool forward(bool first) override;

        bool backward() override;

    private:

        QUuid uuid;
        QByteArray buffer;

        Sequence<ModelObject*> getLinkedItems(const QUuid &seed) const;

        Sequence<ModelObject*> getRemoveItems() const;

        bool anyNeedRebuild(const Sequence<ModelObject*> &sequence) const;
    };

}
