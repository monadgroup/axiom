#include "HistoryOperation.h"

#include "../../util.h"
#include "AddNodeOperation.h"
#include "CreateNodeOperation.h"
#include "DeleteNodeOperation.h"
#include "MoveNodeOperation.h"
#include "SizeNodeOperation.h"
#include "RenameNodeOperation.h"
#include "ShowHideControlNameOperation.h"
#include "MoveControlOperation.h"
#include "SizeControlOperation.h"
#include "ConnectControlsOperation.h"
#include "DisconnectControlsOperation.h"
#include "ChangeNumValueOperation.h"
#include "ChangeNumModeOperation.h"
#include "ChangeCodeOperation.h"

using namespace AxiomModel;

QString HistoryOperation::typeToString(AxiomModel::HistoryOperation::Type type) {
    switch (type) {
        case Type::ADD_NODE: return "Add Node";
        case Type::CREATE_NODE: return "Create Node";
        case Type::DELETE_NODE: return "Delete Node";
        case Type::MOVE_NODE: return "Move Node";
        case Type::SIZE_NODE: return "Size Node";
        case Type::RENAME_NODE: return "Rename Node";
        case Type::SHOW_HIDE_CONTROL_NAME: return "ShowHide Control Name";
        case Type::MOVE_CONTROL: return "Move Control";
        case Type::SIZE_CONTROL: return "Size Control";
        case Type::CONNECT_CONTROLS: return "Connect Controls";
        case Type::DISCONNECT_CONTROLS: return "Disconnect Controls";
        case Type::CHANGE_NUM_VAL: return "Change Num Val";
        case Type::CHANGE_NUM_MODE: return "Change Num Mode";
        case Type::CHANGE_CODE: return "Change Code";
    }

    unreachable;
}

HistoryOperation::HistoryOperation(bool needsRefresh, ReadLevel level, Type type, bool exec)
    : _needsRefresh(needsRefresh), _level(level), _type(type), _exec(exec) {

}

std::unique_ptr<HistoryOperation> HistoryOperation::deserialize(AxiomModel::HistoryOperation::Type type,
                                                                QDataStream &stream, AxiomModel::Project *project) {
    switch (type) {
        case Type::ADD_NODE:
            return AddNodeOperation::deserialize(stream, project);
        case Type::CREATE_NODE:
            return CreateNodeOperation::deserialize(stream, project);
        case Type::DELETE_NODE:
            return DeleteNodeOperation::deserialize(stream, project);
        case Type::MOVE_NODE:
            return MoveNodeOperation::deserialize(stream, project);
        case Type::SIZE_NODE:
            return SizeNodeOperation::deserialize(stream, project);
        case Type::RENAME_NODE:
            return RenameNodeOperation::deserialize(stream, project);
        case Type::SHOW_HIDE_CONTROL_NAME:
            return ShowHideControlNameOperation::deserialize(stream, project);
        case Type::MOVE_CONTROL:
            return MoveControlOperation::deserialize(stream, project);
        case Type::SIZE_CONTROL:
            return SizeControlOperation::deserialize(stream, project);
        case Type::CONNECT_CONTROLS:
            return ConnectControlsOperation::deserialize(stream, project);
        case Type::DISCONNECT_CONTROLS:
            return DisconnectControlsOperation::deserialize(stream, project);
        case Type::CHANGE_NUM_VAL:
            return ChangeNumValueOperation::deserialize(stream, project);
        case Type::CHANGE_NUM_MODE:
            return ChangeNumModeOperation::deserialize(stream, project);
        case Type::CHANGE_CODE:
            return ChangeCodeOperation::deserialize(stream, project);
    }

    unreachable;
}
