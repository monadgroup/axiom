#include "GridSurface.h"

#include <set>
#include <cmath>

using namespace AxiomModel;

void GridSurface::addItem(std::unique_ptr<GridItem> item) {
    auto ptr = item.get();
    m_items.push_back(std::move(item));

    connect(ptr, &GridItem::removed,
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

bool GridSurface::positionAvailable(QPoint pos, QSize size, const GridItem *ignore) const {
    for (auto dx = 0; dx < size.width(); dx++) {
        for (auto dy = 0; dy < size.height(); dy++) {
            auto checkP = pos + QPoint(dx, dy);
            auto found = grid.find(checkP);
            if (found != grid.end() && found->second != ignore) {
                return false;
            }
        }
    }
    return true;
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

QPoint GridSurface::findNearestPos(QPoint pos, QSize size, const GridItem *ignore) const {
    // breadth-first search to find nearest free area
    std::set<SortedPos> positionQueue;
    std::unordered_set<QPoint> visitedQueue;

    positionQueue.insert(SortedPos(pos, pos));
    visitedQueue.insert(pos);
    while (!positionQueue.empty()) {
        auto oldQueue = std::move(positionQueue);
        positionQueue = std::set<SortedPos>();

        for (const auto &p : oldQueue) {
            if (positionAvailable(p.checkPos, size, ignore)) return p.checkPos;

            QPoint offsets[] = {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)};
            for (const auto &offset : offsets) {
                auto newP = p.checkPos + offset;
                if (visitedQueue.find(newP) == visitedQueue.end()) {
                    positionQueue.insert(SortedPos(newP, pos));
                    visitedQueue.insert(newP);
                }
            }
        }
    }

    // oh no! this should never happen :'(
    return pos;
}

void GridSurface::freeGridRect(QPoint pos, QSize size) {
    for (auto dx = 0; dx < size.width(); dx++) {
        for (auto dy = 0; dy < size.height(); dy++) {
            grid.erase(pos + QPoint(dx, dy));
        }
    }
}

void GridSurface::setGridRect(QPoint pos, QSize size, const GridItem *item) {
    for (auto dx = 0; dx < size.width(); dx++) {
        for (auto dy = 0; dy < size.height(); dy++) {
            grid.emplace(pos + QPoint(dx, dy), item);
        }
    }
}

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
        freeGridRect(item->pos(), item->size());
    }

    auto availableDelta = findAvailableDelta(delta);
    lastDragDelta = availableDelta;
    for (auto &item : selectedItems) {
        item->dragTo(availableDelta);
    }

    for (auto &item : selectedItems) {
        setGridRect(item->pos(), item->size(), item);
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
