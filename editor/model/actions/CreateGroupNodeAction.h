#pragma once

#include <QtCore/QUuid>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#include "Action.h"

namespace AxiomModel {

    class CreateGroupNodeAction : public Action {
    public:
        CreateGroupNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, const QUuid &innerUuid, QPointF innerPan, float innerZoom, ModelRoot *root);

        static std::unique_ptr<CreateGroupNodeAction> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, const QUuid &innerUuid, QPointF innerPan, float innerZoom, ModelRoot *root);

        static std::unique_ptr<CreateGroupNodeAction> create(const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, QPointF innerPan, float innerZoom, ModelRoot *root);

        static std::unique_ptr<CreateGroupNodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first) override;

        void backward() override;

    private:
        QUuid uuid;
        QUuid parentUuid;
        QPoint pos;
        QSize size;
        bool selected;
        QString name;
        QUuid controlsUuid;
        QUuid innerUuid;
        QPointF innerPan;
        float innerZoom;

    };

}
