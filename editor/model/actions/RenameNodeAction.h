#pragma once

#include <QtCore/QString>
#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class RenameNodeAction : public Action {
    public:
        RenameNodeAction(const QUuid &uuid, QString oldName, QString newName, ModelRoot *root);

        static std::unique_ptr<RenameNodeAction> create(const QUuid &uuid, QString oldName, QString newName,
                                                        ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const QString &oldName() const { return _oldName; }

        const QString &newName() const { return _newName; }

    private:
        QUuid _uuid;
        QString _oldName;
        QString _newName;
    };
}
