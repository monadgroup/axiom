#include "Action.h"

#include "../../util.h"
#include "CompositeAction.h"
#include "CreateConnectionAction.h"
#include "CreateControlAction.h"
#include "CreateCustomNodeAction.h"
#include "CreateGroupNodeAction.h"
#include "CreatePortalNodeAction.h"
#include "DeleteObjectAction.h"
#include "ExposeControlAction.h"
#include "GridItemMoveAction.h"
#include "GridItemSizeAction.h"
#include "PasteBufferAction.h"
#include "RenameNodeAction.h"
#include "SetCodeAction.h"
#include "SetNumModeAction.h"
#include "SetNumValueAction.h"
#include "SetShowNameAction.h"

using namespace AxiomModel;

Action::Action(AxiomModel::Action::ActionType actionType, AxiomModel::ModelRoot *root)
    : _actionType(actionType), _root(root) {}

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
    case ActionType::RENAME_CONTROL:
        return "Rename Control";
    case ActionType::RENAME_NODE:
        return "Rename Node";
    case ActionType::SET_CODE:
        return "Set Code";
    case ActionType::CREATE_CONTROL:
        return "Create Control";
    case ActionType::SET_NUM_MODE:
        return "Change Display Mode";
    case ActionType::SET_NUM_VALUE:
        return "Change Value";
    case ActionType::SET_SHOW_NAME:
        return "Show/Hide Name";
    case ActionType::EXPOSE_CONTROL:
        return "Expose Control";
    case ActionType::PASTE_BUFFER:
        return "Paste";
    case ActionType::UNEXPOSE_CONTROL:
        return "Unexpose Control";
    case ActionType::ADD_GRAPH_POINT:
        return "Add Graph Point";
    case ActionType::DELETE_GRAPH_POINT:
        return "Delete Graph Point";
    case ActionType::MOVE_GRAPH_POINT:
        return "Move Graph Point";
    case ActionType::SET_GRAPH_TAG:
        return "Set Graph Tag";
    case ActionType::SET_GRAPH_TENSION:
        return "Set Graph Tension";
    case ActionType::SET_NUM_RANGE:
        return "Set Num Range";
    }

    unreachable;
}
