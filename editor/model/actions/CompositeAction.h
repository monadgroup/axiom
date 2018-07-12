#pragma once

#include <vector>

#include "Action.h"

namespace AxiomModel {

    class CompositeAction : public Action {
    public:
        CompositeAction(std::vector<std::unique_ptr<Action>> actions, ModelRoot *root);

        static std::unique_ptr<CompositeAction> create(std::vector<std::unique_ptr<Action>> actions, ModelRoot *root);

        static std::unique_ptr<CompositeAction> deserialize(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        std::vector<std::unique_ptr<Action>> &actions() { return _actions; }

        const std::vector<std::unique_ptr<Action>> &actions() const { return _actions; }

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

    private:
        std::vector<std::unique_ptr<Action>> _actions;
    };
}
