#include "CompositeAction.h"

using namespace AxiomModel;

CompositeAction::CompositeAction(std::vector<std::unique_ptr<AxiomModel::Action>> actions,
                                 AxiomModel::ModelRoot *root)
    : Action(ActionType::COMPOSITE, root), _actions(std::move(actions)) {
}

std::unique_ptr<CompositeAction> CompositeAction::create(std::vector<std::unique_ptr<AxiomModel::Action>> actions,
                                                         AxiomModel::ModelRoot *root) {
    return std::make_unique<CompositeAction>(std::move(actions), root);
}

std::unique_ptr<CompositeAction> CompositeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    uint32_t actionCount; stream >> actionCount;

    std::vector<std::unique_ptr<Action>> actions;
    actions.reserve(actionCount);

    for (uint32_t i = 0; i < actionCount; i++) {
        actions.push_back(Action::deserialize(stream, root));
    }

    return create(std::move(actions), root);
}

void CompositeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);
    stream << (uint32_t) _actions.size();
    for (const auto &action : _actions) {
        action->serialize(stream);
    }
}

bool CompositeAction::needsRebuild() const {
    for (const auto &action : _actions) {
        if (action->needsRebuild()) return true;
    }
    return false;
}

void CompositeAction::forward(bool first) {
    for (const auto &action : _actions) {
        action->forward(first);
    }
}

void CompositeAction::backward() {
    for (auto i = _actions.end() - 1; i >= _actions.begin(); i--) {
        (*i)->backward();
    }
}
