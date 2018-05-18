#include "SetNumModeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

SetNumModeAction::SetNumModeAction(const QUuid &uuid, AxiomModel::NumControl::DisplayMode beforeMode,
                                   AxiomModel::NumControl::DisplayMode afterMode, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_NUM_MODE, root), uuid(uuid), beforeMode(beforeMode), afterMode(afterMode) {
}

std::unique_ptr<SetNumModeAction> SetNumModeAction::create(const QUuid &uuid,
                                                           AxiomModel::NumControl::DisplayMode beforeMode,
                                                           AxiomModel::NumControl::DisplayMode afterMode,
                                                           AxiomModel::ModelRoot *root) {
    return std::make_unique<SetNumModeAction>(uuid, beforeMode, afterMode, root);
}

std::unique_ptr<SetNumModeAction> SetNumModeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid; stream >> uuid;
    uint8_t beforeModeInt; stream >> beforeModeInt;
    uint8_t afterModeInt; stream >> afterModeInt;

    return create(uuid, (NumControl::DisplayMode) beforeModeInt, (NumControl::DisplayMode) afterModeInt, root);
}

void SetNumModeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << (uint8_t) beforeMode;
    stream << (uint8_t) afterMode;
}

bool SetNumModeAction::forward(bool) {
    find<NumControl*>(root()->controls(), uuid)->setDisplayMode(afterMode);
    return false;
}

bool SetNumModeAction::backward() {
    find<NumControl*>(root()->controls(), uuid)->setDisplayMode(beforeMode);
    return false;
}
