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

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &surfaceUuid() const { return _surfaceUuid; }

        const bool &isBufferFormatted() const { return _isBufferFormatted; }

        const QByteArray &buffer() const { return _buffer; }

        const QVector<QUuid> &usedUuids() const { return _usedUuids; }

        const QPoint &center() const { return _center; }

    private:
        QUuid _surfaceUuid;
        bool _isBufferFormatted;
        QByteArray _buffer;
        QVector<QUuid> _usedUuids;
        QPoint _center;
    };
}
