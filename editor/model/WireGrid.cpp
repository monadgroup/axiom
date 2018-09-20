#include "WireGrid.h"

#include <algorithm>

#include "../util.h"

using namespace AxiomModel;

inline uint qHash(const QPoint &key) {
    return qHash(QPair(key.x(), key.y()));
}

void WireGrid::addPoint(QPoint point, AxiomModel::ConnectionWire *wire, Direction direction) {
    auto index = cells.find(point);
    if (index == cells.end()) {
        CellLists lists;
        switch (direction) {
        case Direction::HORIZONTAL:
            lists.horizontal.push_back(wire);
            break;
        case Direction::VERTICAL:
            lists.vertical.push_back(wire);
            break;
        }
        cells[point] = std::move(lists);
    } else {
        std::vector<ConnectionWire *> *collection;
        switch (direction) {
        case Direction::HORIZONTAL:
            collection = &index.value().horizontal;
            break;
        case Direction::VERTICAL:
            collection = &index.value().vertical;
            break;
        }

        // make sure it isn't already in the collection
        auto currentIndex = std::find(collection->begin(), collection->end(), wire);
        if (currentIndex != collection->end()) return;

        collection->push_back(wire);
    }

    _isDirty = true;
}

void WireGrid::removePoint(QPoint point, AxiomModel::ConnectionWire *wire) {
    auto index = cells.find(point);
    if (index == cells.end()) return;

    auto &horizontal = index.value().horizontal;
    auto &vertical = index.value().vertical;

    auto horizontalIndex = std::find(horizontal.begin(), horizontal.end(), wire);
    if (horizontalIndex != horizontal.end()) horizontal.erase(horizontalIndex);
    auto verticalIndex = std::find(vertical.begin(), vertical.end(), wire);
    if (verticalIndex != vertical.end()) vertical.erase(verticalIndex);

    if (horizontal.empty() && vertical.empty()) {
        cells.erase(index);
    }

    _isDirty = true;
}

void WireGrid::addRegion(QRect region, AxiomModel::ConnectionWire *wire) {
    Direction direction;
    if (region.top() == region.bottom())
        direction = Direction::HORIZONTAL;
    else if (region.left() == region.right())
        direction = Direction::VERTICAL;
    else
        unreachable;

    for (auto x = region.left(); x <= region.right(); x++) {
        for (auto y = region.top(); y <= region.bottom(); y++) {
            addPoint(QPoint(x, y), wire, direction);
        }
    }
}

void WireGrid::removeRegion(QRect region, AxiomModel::ConnectionWire *wire) {
    for (auto x = region.left(); x <= region.right(); x++) {
        for (auto y = region.top(); y <= region.bottom(); y++) {
            removePoint(QPoint(x, y), wire);
        }
    }
}

LineIndex WireGrid::getRegionIndex(QRect region, AxiomModel::ConnectionWire *wire) {
    Direction direction;
    if (region.top() == region.bottom())
        direction = Direction::HORIZONTAL;
    else if (region.left() == region.right())
        direction = Direction::VERTICAL;
    else
        unreachable;

    // return the biggest index in the list of cells
    size_t biggestCount = 0;
    size_t biggestIndex = 0;

    for (auto x = region.left(); x <= region.right(); x++) {
        for (auto y = region.top(); y <= region.bottom(); y++) {
            auto cellWires = cells.find(QPoint(x, y));
            if (cellWires == cells.end()) continue;

            std::vector<ConnectionWire *> *collection;
            switch (direction) {
            case Direction::HORIZONTAL:
                collection = &cellWires.value().horizontal;
                break;
            case Direction::VERTICAL:
                collection = &cellWires.value().vertical;
                break;
            }

            if (collection->size() > biggestCount) biggestCount = collection->size();

            auto thisIter = std::find(collection->begin(), collection->end(), wire);
            if (thisIter == collection->end()) continue;

            auto thisIndex = (size_t) std::distance(collection->begin(), thisIter);
            if (thisIndex > biggestIndex) biggestIndex = thisIndex;
        }
    }

    return {biggestCount, biggestIndex};
}

void WireGrid::tryFlush() {
    if (_isDirty) {
        gridChanged.trigger();
        _isDirty = false;
    }
}
