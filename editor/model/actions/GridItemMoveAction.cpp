#include "GridItemMoveAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../grid/GridItem.h"

using namespace AxiomModel;

GridItemMoveAction::GridItemMoveAction(const QUuid &uuid, QPoint beforePos, QPoint afterPos,
                                       AxiomModel::ModelRoot *root)
    : Action(ActionType::MOVE_GRID_ITEM, root), _uuid(uuid), _beforePos(beforePos), _afterPos(afterPos) {}

std::unique_ptr<GridItemMoveAction> GridItemMoveAction::create(const QUuid &uuid, QPoint beforePos, QPoint afterPos,
                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<GridItemMoveAction>(uuid, beforePos, afterPos, root);
}

void GridItemMoveAction::forward(bool, std::vector<QUuid> &) {
    find(AxiomCommon::dynamicCast<GridItem *>(root()->pool().sequence().sequence()), _uuid)->setPos(_afterPos);
}

void GridItemMoveAction::backward(std::vector<QUuid> &) {
    find(AxiomCommon::dynamicCast<GridItem *>(root()->pool().sequence().sequence()), _uuid)->setPos(_beforePos);
}
