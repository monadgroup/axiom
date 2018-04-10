#include "HistoryOperation.h"

#include "AddNodeOperation.h"
#include "DeleteNodeOperation.h"
#include "../../util.h"

using namespace AxiomModel;

HistoryOperation::HistoryOperation(bool needsRefresh, Type type) : _needsRefresh(needsRefresh), _type(type) {

}

std::unique_ptr<HistoryOperation> HistoryOperation::deserialize(AxiomModel::HistoryOperation::Type type,
                                                                QDataStream &stream, AxiomModel::Project *project) {
    switch (type) {
        case Type::ADD_NODE:
            return AddNodeOperation::deserialize(stream, project);
        case Type::DELETE_NODE:
            return DeleteNodeOperation::deserialize(stream, project);
    }

    unreachable;
}
