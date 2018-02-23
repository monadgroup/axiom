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

void GridSurface::cloneTo(GridSurface *surface) const {
    for (const auto &item : m_items) {
        surface->addItem(std::move(item->clone(surface, item->pos(), item->size())));
    }
}

void GridSurface::doRuntimeUpdate() {
    for (const auto &item : m_items) {
        item->doRuntimeUpdate();
    }
}

void GridSurface::deleteSelectedItems() {
    while (!m_selectedItems.empty()) {
        m_selectedItems[0]->remove();
    }
    emit hasSelectionChanged(false);
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
    flushGrid();

    auto loc = std::find(m_selectedItems.begin(), m_selectedItems.end(), item);
    if (loc != m_selectedItems.end()) {
        m_selectedItems.erase(loc);
    }

    for (auto i = m_items.begin(); i < m_items.end(); i++) {
        if (i->get() == item) {
            m_items.erase(i);
            break;
        }
    }

    if (!hasSelection()) emit hasSelectionChanged(false);
}

void GridSurface::selectItem(GridItem *item, bool exclusive) {
    if (exclusive) {
        for (const auto &im : m_items) {
            if (im.get() != item) im->deselect();
        }
    }

    auto hadSelection = hasSelection();
    if (std::find(m_selectedItems.begin(), m_selectedItems.end(), item) == m_selectedItems.end()) {
        m_selectedItems.push_back(item);
    }
    if (!hadSelection) emit hasSelectionChanged(true);
}

void GridSurface::deselectItem(GridItem *item) {
    auto loc = std::find(m_selectedItems.begin(), m_selectedItems.end(), item);
    if (loc != m_selectedItems.end()) {
        m_selectedItems.erase(loc);
    }
    if (!hasSelection()) emit hasSelectionChanged(false);
}

void GridSurface::startDragging() {
    lastDragDelta = QPoint(0, 0);
    for (auto &item : m_selectedItems) {
        item->startDragging();
    }
}

void GridSurface::dragTo(QPoint delta) {
    if (delta == lastDragDelta) return;

    for (auto &item : m_selectedItems) {
        grid.setRect(item->pos(), item->size(), nullptr);
    }

    auto availableDelta = findAvailableDelta(delta);
    lastDragDelta = availableDelta;
    for (auto &item : m_selectedItems) {
        item->dragTo(availableDelta);
    }

    for (auto &item : m_selectedItems) {
        grid.setRect(item->pos(), item->size(), item);
    }
    flushGrid();
}

void GridSurface::finishDragging() {
    for (auto &item : m_selectedItems) {
        item->finishDragging();
    }
}

void GridSurface::flushGrid() {
    emit gridChanged();
}

bool GridSurface::isAllDragAvailable(QPoint delta) {
    for (auto &item : m_selectedItems) {
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
    return lastDragDelta;
}
