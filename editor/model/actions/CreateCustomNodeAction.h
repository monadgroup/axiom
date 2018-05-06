#pragma once

#include <QtCore/QUuid>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#include "Action.h"

namespace AxiomModel {

    class CreateCustomNodeAction : public Action {
    public:
        CreateCustomNodeAction(const QUuid &parentId, const QUuid &id, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, QString code, ModelRoot *root);

        static std::unique_ptr<CreateCustomNodeAction> create(const QUuid &parentId, const QUuid &id, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, QString code, ModelRoot *root);

        static std::unique_ptr<CreateCustomNodeAction> create(const QUuid &parentId, QPoint pos, QSize size, bool selected, QString name, QString code, ModelRoot *root);

        static std::unique_ptr<CreateCustomNodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward() const override;

        void backward() const override;

    private:
        QUuid parentId;
        QUuid id;
        QPoint pos;
        QSize size;
        bool selected;
        QString name;
        QUuid controlsId;
        QString code;
    };

}
