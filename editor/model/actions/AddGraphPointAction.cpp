#include "AddGraphPointAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GraphControl.h"

using namespace AxiomModel;

AddGraphPointAction::AddGraphPointAction(const QUuid &controlUuid, uint8_t index, float time, float val, float tension,
                                         AxiomModel::ModelRoot *root)
    : Action(ActionType::ADD_GRAPH_POINT, root), _controlUuid(controlUuid), _index(index), _time(time), _val(val),
      _tension(tension) {}

std::unique_ptr<AddGraphPointAction> AddGraphPointAction::create(const QUuid &controlUuid, uint8_t index, float time,
                                                                 float val, float tension,
                                                                 AxiomModel::ModelRoot *root) {
    return std::make_unique<AddGraphPointAction>(controlUuid, time, index, val, tension, root);
}

void AddGraphPointAction::forward(bool, std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->insertPoint(_index, _time, _val, _tension);
}

void AddGraphPointAction::backward(std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->removePoint(_index);
}
