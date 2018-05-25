#pragma once

#include <QtCore/QUuid>
#include <QtCore/QPoint>
#include <QtCore/QByteArray>
#include <QtCore/QVector>

#include "Action.h"

namespace AxiomModel {

    class PasteBufferAction : public Action {
    public:
        PasteBufferAction(const QUuid &surfaceUuid, bool isBufferFormatted, QByteArray buffer, QVector<QUuid> usedUuids, QPoint center, ModelRoot *root);

        static std::unique_ptr<PasteBufferAction> create(const QUuid &surfaceUuid, bool isBufferFormatted, QByteArray buffer, QVector<QUuid> usedUuids, QPoint center, ModelRoot *root);

        static std::unique_ptr<PasteBufferAction> create(const QUuid &surfaceUuid, QByteArray buffer, QPoint center, ModelRoot *root);

        static std::unique_ptr<PasteBufferAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        bool forward(bool first) override;

        bool backward() override;

    private:
        QUuid surfaceUuid;
        bool isBufferFormatted;
        QByteArray buffer;
        QVector<QUuid> usedUuids;
        QPoint center;
    };

}
