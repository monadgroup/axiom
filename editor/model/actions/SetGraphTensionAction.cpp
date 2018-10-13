#include "SetGraphTensionAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GraphControl.h"

using namespace AxiomModel;

SetGraphTensionAction::SetGraphTensionAction(const QUuid &controlUuid, uint8_t index, float oldTension,
                                             float newTension, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_GRAPH_TENSION, root), _controlUuid(controlUuid), _index(index), _oldTension(oldTension),
      _newTension(newTension) {}

std::unique_ptr<SetGraphTensionAction> SetGraphTensionAction::create(const QUuid &controlUuid, uint8_t index,
                                                                     float oldTension, float newTension,
                                                                     AxiomModel::ModelRoot *root) {
    return std::make_unique<SetGraphTensionAction>(controlUuid, index, oldTension, newTension, root);
}

void SetGraphTensionAction::forward(bool, std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence().sequence(), _controlUuid)->setCurveTension(_index, _newTension);
}

void SetGraphTensionAction::backward(std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence().sequence(), _controlUuid)->setCurveTension(_index, _oldTension);
}
