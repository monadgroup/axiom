#include "Action.h"

#include "CompositeAction.h"
#include "DeleteObjectAction.h"
#include "CreateCustomNodeAction.h"
#include "CreateGroupNodeAction.h"
#include "CreatePortalNodeAction.h"
#include "CreateConnectionAction.h"
#include "GridItemMoveAction.h"
#include "GridItemSizeAction.h"
#include "RenameNodeAction.h"
#include "SetCodeAction.h"
#include "../../util.h"

using namespace AxiomModel;

Action::Action(AxiomModel::Action::ActionType actionType, AxiomModel::ModelRoot *root)
    : _actionType(actionType), _root(root) {
}

QString Action::typeToString(AxiomModel::Action::ActionType type) {
    switch (type) {
        case ActionType::NONE:
            return "";
        case ActionType::COMPOSITE:
            return "Composite";
        case ActionType::DELETE_OBJECT:
            return "Delete Object";
        case ActionType::CREATE_CUSTOM_NODE:
            return "Create Custom Node";
        case ActionType::CREATE_GROUP_NODE:
            return "Create Group Node";
        case ActionType::CREATE_PORTAL_NODE:
            return "Create Portal Node";
        case ActionType::CREATE_CONNECTION:
            return "Connect Controls";
        case ActionType::MOVE_GRID_ITEM:
            return "Move Grid Item";
        case ActionType::SIZE_GRID_ITEM:
            return "Size Grid Item";
        case ActionType::RENAME_NODE:
            return "Rename Node";
        case ActionType::SET_CODE:
            return "Set Code";
    }

    unreachable;
}

std::unique_ptr<Action> Action::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    uint8_t actionTypeInt;
    stream >> actionTypeInt;

    auto actionType = (ActionType) actionTypeInt;
    assert(actionType != ActionType::NONE);

    switch (actionType) {
        case ActionType::COMPOSITE:
            return CompositeAction::deserialize(stream, root);
        case ActionType::DELETE_OBJECT:
            return DeleteObjectAction::deserialize(stream, root);
        case ActionType::CREATE_CUSTOM_NODE:
            return CreateCustomNodeAction::deserialize(stream, root);
        case ActionType::CREATE_GROUP_NODE:
            return CreateGroupNodeAction::deserialize(stream, root);
        case ActionType::CREATE_PORTAL_NODE:
            return CreatePortalNodeAction::deserialize(stream, root);
        case ActionType::CREATE_CONNECTION:
            return CreateConnectionAction::deserialize(stream, root);
        case ActionType::MOVE_GRID_ITEM:
            return GridItemMoveAction::deserialize(stream, root);
        case ActionType::SIZE_GRID_ITEM:
            return GridItemSizeAction::deserialize(stream, root);
        case ActionType::RENAME_NODE:
            return RenameNodeAction::deserialize(stream, root);
        case ActionType::SET_CODE:
            return SetCodeAction::deserialize(stream, root);
        default:
        unreachable;
    }
}

void Action::serialize(QDataStream &stream) const {
    stream << (uint8_t) _actionType;
}
