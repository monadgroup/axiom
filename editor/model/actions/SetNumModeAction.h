#pragma once

#include <QtCore/QUuid>

#include "Action.h"
#include "../objects/NumControl.h"

namespace AxiomModel {

    class SetNumModeAction : public Action {
    public:
        SetNumModeAction(const QUuid &uuid, NumControl::DisplayMode beforeMode, NumControl::DisplayMode afterMode,
                         ModelRoot *root);

        static std::unique_ptr<SetNumModeAction>
        create(const QUuid &uuid, NumControl::DisplayMode beforeMode, NumControl::DisplayMode afterMode,
               ModelRoot *root);

        static std::unique_ptr<SetNumModeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        bool forward(bool first) override;

        bool backward() override;

    private:

        QUuid uuid;
        NumControl::DisplayMode beforeMode;
        NumControl::DisplayMode afterMode;
    };

}
