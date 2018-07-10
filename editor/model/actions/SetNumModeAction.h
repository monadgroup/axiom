#pragma once

#include <QtCore/QUuid>

#include "../objects/NumControl.h"
#include "Action.h"

namespace AxiomModel {

    class SetNumModeAction : public Action {
    public:
        SetNumModeAction(const QUuid &uuid, NumControl::DisplayMode beforeMode, NumControl::DisplayMode afterMode,
                         ModelRoot *root);

        static std::unique_ptr<SetNumModeAction> create(const QUuid &uuid, NumControl::DisplayMode beforeMode,
                                                        NumControl::DisplayMode afterMode, ModelRoot *root);

        static std::unique_ptr<SetNumModeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

    private:
        QUuid uuid;
        NumControl::DisplayMode beforeMode;
        NumControl::DisplayMode afterMode;
    };
}
