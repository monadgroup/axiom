#pragma once

#include <QtCore/QUuid>
#include <QtCore/QString>

#include "Action.h"

namespace AxiomModel {

    class RenameNodeAction : public Action {
    public:
        RenameNodeAction(const QUuid &uuid, QString oldName, QString newName, ModelRoot *root);

        static std::unique_ptr<RenameNodeAction> create(const QUuid &uuid, QString oldName, QString newName, ModelRoot *root);

        static std::unique_ptr<RenameNodeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first) override;

        void backward() override;

    private:

        QUuid uuid;
        QString oldName;
        QString newName;
    };

}
