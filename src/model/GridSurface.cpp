#include "GridSurface.h"

using namespace AxiomModel;

GridSurface::GridSurface(QPoint minRect, QPoint maxRect) : grid(Grid<GridItem>(minRect, maxRect)) {

}

void GridSurface::addItem(std::unique_ptr<GridItem> item) {
    auto ptr = item.get();
    m_items.push_back(std::move(item));

    connect(ptr, &GridItem::cleanup,
            this, [this, ptr]() { removeItem(ptr); });
    connect(ptr, &GridItem::selected,
            this, [this, ptr](bool exclusive) { selectItem(ptr, exclusive); });
    connect(ptr, &GridItem::deselected,
            this, [this, ptr]() { deselectItem(ptr); });

    connect(ptr, &GridItem::startedDragging,
            this, &GridSurface::startDragging);
    connect(ptr, &GridItem::draggedTo,
            this, &GridSurface::dragTo);
    connect(ptr, &GridItem::finishedDragging,
            this, &GridSurface::finishDragging);

    emit itemAdded(ptr);
}

class SortedPos {
public:
    QPoint checkPos;
    QPoint basePos;

    SortedPos(QPoint checkPos, QPoint basePos) : checkPos(checkPos), basePos(basePos) {}

    float dist() const {
        return std::sqrt((float) checkPos.x() * checkPos.x() + checkPos.y() * checkPos.y());
    }

    bool operator<(const SortedPos &other) const {
        return dist() < other.dist();
    }
};

void GridSurface::deleteSelectedItems() {
    while (!selectedItems.empty()) {
        selectedItems[0]->remove();
    }
}

void GridSurface::selectAll() {
    for (const auto &item : m_items) {
        item->select(false);
    }
}

void GridSurface::deselectAll() {
    for (const auto &item : m_items) {
        item->deselect();
    }
}

void GridSurface::removeItem(GridItem *item) {
    grid.setRect(item->pos(), item->size(), nullptr);

    auto loc = std::find(selectedItems.begin(), selectedItems.end(), item);
    if (loc != selectedItems.end()) {
        selectedItems.erase(loc);
    }

    for (auto i = m_items.begin(); i < m_items.end(); i++) {
        if (i->get() == item) {
            m_items.erase(i);
            break;
        }
    }
}

void GridSurface::selectItem(GridItem *item, bool exclusive) {
    if (exclusive) {
        for (const auto &im : m_items) {
            if (im.get() != item) im->deselect();
        }
    }

    if (std::find(selectedItems.begin(), selectedItems.end(), item) == selectedItems.end()) {
        selectedItems.push_back(item);
    }
}

void GridSurface::deselectItem(GridItem *item) {
    auto loc = std::find(selectedItems.begin(), selectedItems.end(), item);
    if (loc != selectedItems.end()) {
        selectedItems.erase(loc);
    }
}

void GridSurface::startDragging() {
    lastDragDelta = QPoint(0, 0);
    for (auto &item : selectedItems) {
        item->startDragging();
    }
}

void GridSurface::dragTo(QPoint delta) {
    if (delta == lastDragDelta) return;

    for (auto &item : selectedItems) {
        grid.setRect(item->pos(), item->size(), nullptr);
    }

    auto availableDelta = findAvailableDelta(delta);
    lastDragDelta = availableDelta;
    for (auto &item : selectedItems) {
        item->dragTo(availableDelta);
    }

    for (auto &item : selectedItems) {
        grid.setRect(item->pos(), item->size(), item);
    }
}

void GridSurface::finishDragging() {
    for (auto &item : selectedItems) {
        item->finishDragging();
    }
}

bool GridSurface::isAllDragAvailable(QPoint delta) {
    for (auto &item : selectedItems) {
        if (!item->isDragAvailable(delta)) return false;
    }
    return true;
}

QPoint GridSurface::findAvailableDelta(QPoint delta) {
    if (isAllDragAvailable(delta)) return delta;
    auto xDelta = QPoint(delta.x(), lastDragDelta.y());
    if (isAllDragAvailable(xDelta)) return xDelta;
    auto yDelta = QPoint(lastDragDelta.x(), delta.y());
    if (isAllDragAvailable(yDelta)) return yDelta;
    return {0, 0};
}
