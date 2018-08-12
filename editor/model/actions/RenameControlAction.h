#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class RenameControlAction : public Action {
    public:
        RenameControlAction(const QUuid &uuid, QString oldName, QString newName, ModelRoot *root);

        static std::unique_ptr<RenameControlAction> create(const QUuid &uuid, QString oldName, QString newName,
                                                           ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &uuid() const { return _uuid; }

        const QString &oldName() const { return _oldName; }

        const QString &newName() const { return _newName; }

    private:
        QUuid _uuid;
        QString _oldName;
        QString _newName;
    };
}
