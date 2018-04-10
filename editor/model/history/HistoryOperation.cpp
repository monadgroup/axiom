#include "HistoryOperation.h"

#include "AddNodeOperation.h"
#include "DeleteNodeOperation.h"
#include "../../util.h"
#include "MoveNodeOperation.h"

using namespace AxiomModel;

HistoryOperation::HistoryOperation(bool needsRefresh, Type type, bool exec) : _needsRefresh(needsRefresh), _type(type), _exec(exec) {

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
    }

    unreachable;
}
