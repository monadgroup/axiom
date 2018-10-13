#include "SetNumRangeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/NumControl.h"

using namespace AxiomModel;

SetNumRangeAction::SetNumRangeAction(const QUuid &uuid, float beforeMin, float beforeMax, uint32_t beforeStep,
                                     float afterMin, float afterMax, uint32_t afterStep, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_NUM_RANGE, root), _uuid(uuid), _beforeMin(beforeMin), _beforeMax(beforeMax),
      _beforeStep(beforeStep), _afterMin(afterMin), _afterMax(afterMax), _afterStep(afterStep) {}

std::unique_ptr<SetNumRangeAction> SetNumRangeAction::create(const QUuid &uuid, float beforeMin, float beforeMax,
                                                             uint32_t beforeStep, float afterMin, float afterMax,
                                                             uint32_t afterStep, AxiomModel::ModelRoot *root) {
    return std::make_unique<SetNumRangeAction>(uuid, beforeMin, beforeMax, beforeStep, afterMin, afterMax, afterStep,
                                               root);
}

void SetNumRangeAction::forward(bool, std::vector<QUuid> &) {
    find<NumControl *>(root()->controls().sequence(), _uuid)->setRange(_afterMin, _afterMax, _afterStep);
}

void SetNumRangeAction::backward(std::vector<QUuid> &) {
    find<NumControl *>(root()->controls().sequence(), _uuid)->setRange(_beforeMin, _beforeMax, _beforeStep);
}
