#pragma once

#include <QtCore/QUuid>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#include "Action.h"

namespace AxiomModel {

    class CreateCustomNodeAction : public Action {
    public:
        CreateCustomNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name, const QUuid &controlsUuid, ModelRoot *root);

        static std::unique_ptr<CreateCustomNodeAction> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name, const QUuid &controlsUuid, ModelRoot *root);

        static std::unique_ptr<CreateCustomNodeAction> create(const QUuid &parentUuid, QPoint pos, QString name, ModelRoot *root);

        static std::unique_ptr<CreateCustomNodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first) override;

        void backward() override;

    private:
        QUuid uuid;
        QUuid parentUuid;
        QPoint pos;
        QString name;
        QUuid controlsUuid;
    };

}
