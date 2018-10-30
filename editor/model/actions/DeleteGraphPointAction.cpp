#include "DeleteGraphPointAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GraphControl.h"

using namespace AxiomModel;

DeleteGraphPointAction::DeleteGraphPointAction(const QUuid &controlUuid, uint8_t index, float time, float val,
                                               float tension, uint8_t state, AxiomModel::ModelRoot *root)
    : Action(ActionType::DELETE_GRAPH_POINT, root), _controlUuid(controlUuid), _index(index), _time(time), _val(val),
      _tension(tension), _state(state) {}

std::unique_ptr<DeleteGraphPointAction> DeleteGraphPointAction::create(const QUuid &controlUuid, uint8_t index,
                                                                       float time, float val, float tension,
                                                                       uint8_t state, AxiomModel::ModelRoot *root) {
    return std::make_unique<DeleteGraphPointAction>(controlUuid, index, time, val, tension, state, root);
}

void DeleteGraphPointAction::forward(bool) {
    find(AxiomCommon::dynamicCast<GraphControl *>(root()->pool().sequence().sequence()), _controlUuid)
        ->removePoint(_index);
}

void DeleteGraphPointAction::backward() {
    find(AxiomCommon::dynamicCast<GraphControl *>(root()->pool().sequence().sequence()), _controlUuid)
        ->insertPoint((uint8_t)(_index - 1), _time, _val, _tension, _state);
}
