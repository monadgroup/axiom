#pragma once

#include <QtCore/QUuid>

#include "../objects/Control.h"
#include "Action.h"

namespace AxiomModel {

    class CreateControlAction : public Action {
    public:
        CreateControlAction(const QUuid &uuid, const QUuid &parentUuid, Control::ControlType type, QString name,
                            ModelRoot *root);

        static std::unique_ptr<CreateControlAction> create(const QUuid &uuid, const QUuid &parentUuid,
                                                           Control::ControlType type, QString name, ModelRoot *root);

        static std::unique_ptr<CreateControlAction> create(const QUuid &parentUuid, Control::ControlType type,
                                                           QString name, ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &uuid() const { return _uuid; }

        const QUuid &parentUuid() const { return _parentUuid; }

        const Control::ControlType &type() const { return _type; }

        const QString &name() const { return _name; }

    private:
        QUuid _uuid;
        QUuid _parentUuid;
        Control::ControlType _type;
        QString _name;
    };
}
