#include "SetGraphTagAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GraphControl.h"

using namespace AxiomModel;

SetGraphTagAction::SetGraphTagAction(const QUuid &controlUuid, uint8_t index, uint8_t oldTag, uint8_t newTag,
                                     AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_GRAPH_TAG, root), _controlUuid(controlUuid), _index(index), _oldTag(oldTag),
      _newTag(newTag) {}

std::unique_ptr<SetGraphTagAction> SetGraphTagAction::create(const QUuid &controlUuid, uint8_t index, uint8_t oldTag,
                                                             uint8_t newTag, AxiomModel::ModelRoot *root) {
    return std::make_unique<SetGraphTagAction>(controlUuid, index, oldTag, newTag, root);
}

void SetGraphTagAction::forward(bool, std::vector<QUuid> &) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->setPointTag(_index, _newTag);
}

void SetGraphTagAction::backward(std::vector<QUuid> &compileItems) {
    find<GraphControl *>(root()->pool().sequence(), _controlUuid)->setPointTag(_index, _oldTag);
}
