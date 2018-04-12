#include "HistoryOperation.h"

#include "../../util.h"
#include "AddNodeOperation.h"
#include "DeleteNodeOperation.h"
#include "MoveNodeOperation.h"
#include "SizeNodeOperation.h"
#include "RenameNodeOperation.h"
#include "ShowHideControlNameOperation.h"
#include "MoveControlOperation.h"
#include "SizeControlOperation.h"
#include "ConnectControlsOperation.h"
#include "DisconnectControlsOperation.h"

using namespace AxiomModel;

HistoryOperation::HistoryOperation(bool needsRefresh, Type type, bool exec) : _needsRefresh(needsRefresh), _type(type),
                                                                              _exec(exec) {

}

std::unique_ptr<HistoryOperation> HistoryOperation::deserialize(AxiomModel::HistoryOperation::Type type,
                                                                QDataStream &stream, AxiomModel::Project *project) {
    switch (type) {
        case Type::ADD_NODE:
            return AddNodeOperation::deserialize(stream, project);
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
    }

    unreachable;
}
