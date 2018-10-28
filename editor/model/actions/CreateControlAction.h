#pragma once

#include <QtCore/QUuid>

#include "../objects/Control.h"
#include "Action.h"

namespace AxiomModel {

    class CompositeAction;

    class CreateControlAction : public Action {
    public:
        CreateControlAction(const QUuid &uuid, const QUuid &parentUuid, Control::ControlType type, QString name,
                            QPoint pos, QSize size, bool isWrittenTo, ModelRoot *root);

        static std::unique_ptr<CreateControlAction> create(const QUuid &uuid, const QUuid &parentUuid,
                                                           Control::ControlType type, QString name, QPoint pos,
                                                           QSize size, bool isWrittenTo, ModelRoot *root);

        static std::unique_ptr<CreateControlAction> create(const QUuid &parentUuid, Control::ControlType type,
                                                           QString name, QPoint pos, QSize size, bool isWrittenTo,
                                                           ModelRoot *root);

        static std::unique_ptr<CompositeAction> create(const QUuid &parentUuid, Control::ControlType type, QString name,
                                                       bool isWrittenTo, ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const QUuid &parentUuid() const { return _parentUuid; }

        const Control::ControlType &type() const { return _type; }

        const QString &name() const { return _name; }

        QPoint pos() const { return _pos; }

        QSize size() const { return _size; }

        bool isWrittenTo() const { return _isWrittenTo; }

    private:
        QUuid _uuid;
        QUuid _parentUuid;
        Control::ControlType _type;
        QString _name;
        QPoint _pos;
        QSize _size;
        bool _isWrittenTo;
    };
}
