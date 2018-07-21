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

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &uuid() const { return _uuid; }

        const NumControl::DisplayMode &beforeMode() const { return _beforeMode; }

        const NumControl::DisplayMode &afterMode() const { return _afterMode; }

    private:
        QUuid _uuid;
        NumControl::DisplayMode _beforeMode;
        NumControl::DisplayMode _afterMode;
    };
}
