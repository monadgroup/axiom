#include "GridItemSizeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../grid/GridItem.h"

using namespace AxiomModel;

GridItemSizeAction::GridItemSizeAction(const QUuid &uuid, QRect beforeRect, QRect afterRect,
                                       AxiomModel::ModelRoot *root)
    : Action(ActionType::SIZE_GRID_ITEM, root), uuid(uuid), beforeRect(beforeRect), afterRect(afterRect) {
}

std::unique_ptr<GridItemSizeAction> GridItemSizeAction::create(const QUuid &uuid, QRect beforeRect, QRect afterRect,
                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<GridItemSizeAction>(uuid, beforeRect, afterRect, root);
}

std::unique_ptr<GridItemSizeAction> GridItemSizeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QRect beforeRect;
    stream >> beforeRect;
    QRect afterRect;
    stream >> afterRect;

    return create(uuid, beforeRect, afterRect, root);
}

void GridItemSizeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << beforeRect;
    stream << afterRect;
}

void GridItemSizeAction::forward(bool) {
    find<GridItem *>(root()->pool().sequence(), uuid)->setRect(afterRect);
}

void GridItemSizeAction::backward() {
    find<GridItem *>(root()->pool().sequence(), uuid)->setRect(beforeRect);
}
