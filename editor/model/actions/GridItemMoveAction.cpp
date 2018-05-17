#include "GridItemMoveAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../grid/GridItem.h"

using namespace AxiomModel;

GridItemMoveAction::GridItemMoveAction(const QUuid &uuid, QPoint beforePos, QPoint afterPos,
                                       AxiomModel::ModelRoot *root)
    : Action(ActionType::MOVE_GRID_ITEM, root), uuid(uuid), beforePos(beforePos), afterPos(afterPos) {
}

std::unique_ptr<GridItemMoveAction> GridItemMoveAction::create(const QUuid &uuid, QPoint beforePos, QPoint afterPos,
                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<GridItemMoveAction>(uuid, beforePos, afterPos, root);
}

std::unique_ptr<GridItemMoveAction> GridItemMoveAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QPoint beforePos;
    stream >> beforePos;
    QPoint afterPos;
    stream >> afterPos;

    return create(uuid, beforePos, afterPos, root);
}

void GridItemMoveAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << beforePos;
    stream << afterPos;
}

void GridItemMoveAction::forward(bool) {
    find<GridItem *>(root()->pool().sequence(), uuid)->setPos(afterPos);
}

void GridItemMoveAction::backward() {
    find<GridItem *>(root()->pool().sequence(), uuid)->setPos(beforePos);
}
