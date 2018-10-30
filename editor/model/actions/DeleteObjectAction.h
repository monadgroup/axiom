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

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const QByteArray &buffer() const { return _buffer; }

    private:
        QUuid _uuid;
        QByteArray _buffer;

        std::vector<ModelObject *> getLinkedItems(const QUuid &seed) const;
    };
}
