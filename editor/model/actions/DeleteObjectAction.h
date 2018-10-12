#pragma once

#include <QtCore/QUuid>

#include "Action.h"
#include "common/Sequence.h"

namespace AxiomModel {

    class ModelObject;

    class DeleteObjectAction : public Action {
    public:
        DeleteObjectAction(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> create(const QUuid &uuid, QByteArray buffer,
                                                          AxiomModel::ModelRoot *root);

        static std::unique_ptr<DeleteObjectAction> create(const QUuid &uuid, AxiomModel::ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &uuid() const { return _uuid; }

        const QByteArray &buffer() const { return _buffer; }

    private:
        QUuid _uuid;
        QByteArray _buffer;

        AxiomCommon::BoxedSequence<ModelObject *> getLinkedItems(const QUuid &seed) const;

        AxiomCommon::BoxedSequence<ModelObject *> getRemoveItems() const;
    };
}
