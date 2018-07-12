#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QPoint>
#include <QtCore/QUuid>
#include <QtCore/QVector>

#include "Action.h"

namespace AxiomModel {

    class PasteBufferAction : public Action {
    public:
        PasteBufferAction(const QUuid &surfaceUuid, bool isBufferFormatted, QByteArray buffer, QVector<QUuid> usedUuids,
                          QPoint center, ModelRoot *root);

        static std::unique_ptr<PasteBufferAction> create(const QUuid &surfaceUuid, bool isBufferFormatted,
                                                         QByteArray buffer, QVector<QUuid> usedUuids, QPoint center,
                                                         ModelRoot *root);

        static std::unique_ptr<PasteBufferAction> create(const QUuid &surfaceUuid, QByteArray buffer, QPoint center,
                                                         ModelRoot *root);

        static std::unique_ptr<PasteBufferAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

    private:
        QUuid surfaceUuid;
        bool isBufferFormatted;
        QByteArray buffer;
        QVector<QUuid> usedUuids;
        QPoint center;
    };
}
