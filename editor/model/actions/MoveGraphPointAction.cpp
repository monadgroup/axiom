#include "MoveGraphPointAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GraphControl.h"

using namespace AxiomModel;

MoveGraphPointAction::MoveGraphPointAction(const QUuid &controlUuid, uint8_t index, float oldTime, float oldValue,
                                           float newTime, float newValue, AxiomModel::ModelRoot *root)
    : Action(ActionType::MOVE_GRAPH_POINT, root), _controlUuid(controlUuid), _index(index), _oldTime(oldTime),
      _oldValue(oldValue), _newTime(newTime), _newValue(newValue) {}

std::unique_ptr<MoveGraphPointAction> MoveGraphPointAction::create(const QUuid &controlUuid, uint8_t index,
                                                                   float oldTime, float oldValue, float newTime,
                                                                   float newValue, AxiomModel::ModelRoot *root) {
    return std::make_unique<MoveGraphPointAction>(controlUuid, index, oldTime, oldValue, newTime, newValue, root);
}

void MoveGraphPointAction::forward(bool, std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->movePoint(_index, _newTime, _newValue);
}

void MoveGraphPointAction::backward(std::vector<QUuid> &compileItems) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->movePoint(_index, _oldTime, _oldValue);
}
