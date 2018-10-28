#pragma once

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class CreateGroupNodeAction : public Action {
    public:
        CreateGroupNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                              const QUuid &controlsUuid, const QUuid &innerUuid, ModelRoot *root);

        static std::unique_ptr<CreateGroupNodeAction> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                             QString name, const QUuid &controlsUuid,
                                                             const QUuid &innerUuid, ModelRoot *root);

        static std::unique_ptr<CreateGroupNodeAction> create(const QUuid &parentUuid, QPoint pos, QString name,
                                                             ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const QUuid &parentUuid() const { return _parentUuid; }

        const QPoint &pos() const { return _pos; }

        const QString &name() const { return _name; }

        const QUuid &controlsUuid() const { return _controlsUuid; }

        const QUuid &innerUuid() const { return _innerUuid; }

    private:
        QUuid _uuid;
        QUuid _parentUuid;
        QPoint _pos;
        QString _name;
        QUuid _controlsUuid;
        QUuid _innerUuid;
    };
}
