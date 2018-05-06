#include "Action.h"

#include "DeleteObjectAction.h"
#include "CreateCustomNodeAction.h"
#include "CreateGroupNodeAction.h"
#include "../../util.h"

using namespace AxiomModel;

Action::Action(AxiomModel::Action::ActionType actionType, AxiomModel::ModelRoot *root)
    : _actionType(actionType), _root(root) {

}

std::unique_ptr<Action> Action::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    uint8_t actionTypeInt; stream >> actionTypeInt;

    switch ((ActionType) actionTypeInt) {
        case ActionType::DELETE_OBJECT: return DeleteObjectAction::deserialize(stream, root);
        case ActionType::CREATE_CUSTOM_NODE: return CreateCustomNodeAction::deserialize(stream, root);
        case ActionType::CREATE_GROUP_NODE: return CreateGroupNodeAction::deserialize(stream, root);
    }

    unreachable;
}

void Action::serialize(QDataStream &stream) const {
    stream << (uint8_t) _actionType;
}
