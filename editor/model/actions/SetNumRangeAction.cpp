#include "SetNumRangeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/NumControl.h"

using namespace AxiomModel;

SetNumRangeAction::SetNumRangeAction(const QUuid &uuid, float beforeMin, float beforeMax, float afterMin,
                                     float afterMax, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_NUM_RANGE, root), _uuid(uuid), _beforeMin(beforeMin), _beforeMax(beforeMax),
      _afterMin(afterMin), _afterMax(afterMax) {}

std::unique_ptr<SetNumRangeAction> SetNumRangeAction::create(const QUuid &uuid, float beforeMin, float beforeMax,
                                                             float afterMin, float afterMax,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<SetNumRangeAction>(uuid, beforeMin, beforeMax, afterMin, afterMax, root);
}

void SetNumRangeAction::forward(bool, std::vector<QUuid> &) {
    find<NumControl *>(root()->controls(), _uuid)->setRange(_afterMin, _afterMax);
}

void SetNumRangeAction::backward(std::vector<QUuid> &) {
    find<NumControl *>(root()->controls(), _uuid)->setRange(_beforeMin, _beforeMax);
}
