#include "DeleteGraphPointAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GraphControl.h"

using namespace AxiomModel;

DeleteGraphPointAction::DeleteGraphPointAction(const QUuid &controlUuid, uint8_t index, float time, float val,
                                               float tension, AxiomModel::ModelRoot *root)
    : Action(ActionType::DELETE_GRAPH_POINT, root), _controlUuid(controlUuid), _index(index), _time(time), _val(val),
      _tension(tension) {}

std::unique_ptr<DeleteGraphPointAction> DeleteGraphPointAction::create(const QUuid &controlUuid, uint8_t index,
                                                                       float time, float val, float tension,
                                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<DeleteGraphPointAction>(controlUuid, index, time, val, tension, root);
}

void DeleteGraphPointAction::forward(bool, std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->removePoint(_index);
}

void DeleteGraphPointAction::backward(std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->insertPoint(_index, _time, _val, _tension);
}
