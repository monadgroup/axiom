#include "GridItemSizeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../grid/GridItem.h"

using namespace AxiomModel;

GridItemSizeAction::GridItemSizeAction(const QUuid &uuid, QRect beforeRect, QRect afterRect,
                                       AxiomModel::ModelRoot *root)
    : Action(ActionType::SIZE_GRID_ITEM, root), _uuid(uuid), _beforeRect(beforeRect), _afterRect(afterRect) {}

std::unique_ptr<GridItemSizeAction> GridItemSizeAction::create(const QUuid &uuid, QRect beforeRect, QRect afterRect,
                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<GridItemSizeAction>(uuid, beforeRect, afterRect, root);
}

void GridItemSizeAction::forward(bool, std::vector<QUuid> &) {
    find(AxiomCommon::dynamicCast<GridItem *>(root()->pool().sequence().sequence()), _uuid)->setRect(_afterRect);
}

void GridItemSizeAction::backward(std::vector<QUuid> &) {
    find(AxiomCommon::dynamicCast<GridItem *>(root()->pool().sequence().sequence()), _uuid)->setRect(_beforeRect);
}
