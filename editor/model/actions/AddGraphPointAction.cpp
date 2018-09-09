#include "AddGraphPointAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GraphControl.h"

using namespace AxiomModel;

AddGraphPointAction::AddGraphPointAction(const QUuid &controlUuid, uint8_t index, float time, float val,
                                         AxiomModel::ModelRoot *root)
    : Action(ActionType::ADD_GRAPH_POINT, root), _controlUuid(controlUuid), _index(index), _time(time), _val(val) {}

std::unique_ptr<AddGraphPointAction> AddGraphPointAction::create(const QUuid &controlUuid, uint8_t index, float time,
                                                                 float val, AxiomModel::ModelRoot *root) {
    return std::make_unique<AddGraphPointAction>(controlUuid, index, time, val, root);
}

void AddGraphPointAction::forward(bool, std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->insertPoint(_index, _time, _val, 0, 0);
}

void AddGraphPointAction::backward(std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->removePoint((uint8_t)(_index + 1));
}
