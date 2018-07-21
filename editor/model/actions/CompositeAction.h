#pragma once

#include <vector>

#include "Action.h"

namespace AxiomModel {

    class CompositeAction : public Action {
    public:
        CompositeAction(std::vector<std::unique_ptr<Action>> actions, ModelRoot *root);

        static std::unique_ptr<CompositeAction> create(std::vector<std::unique_ptr<Action>> actions, ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        std::vector<std::unique_ptr<Action>> &actions() { return _actions; }

        const std::vector<std::unique_ptr<Action>> &actions() const { return _actions; }

    private:
        std::vector<std::unique_ptr<Action>> _actions;
    };
}
