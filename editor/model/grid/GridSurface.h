#pragma once

#include <QtCore/QPointF>
#include <QtCore/QString>
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../Pool.h"
#include "Grid.h"
#include "common/Event.h"
#include "common/TrackedObject.h"
#include "common/WatchSequence.h"

namespace AxiomModel {

    class GridItem;

    class GridSurface : public AxiomCommon::TrackedObject {
    public:
        using ItemGrid = Grid<GridItem>;
        using ItemCollection = AxiomCommon::BoxedWatchSequence<GridItem *>;

        AxiomCommon::Event<GridItem *> itemAdded;
        AxiomCommon::Event<bool> hasSelectionChanged;
        AxiomCommon::Event<> gridChanged;

        GridSurface(ItemCollection view, bool deferDirty, QPoint minRect = QPoint(INT_MIN, INT_MIN),
                    QPoint maxRect = QPoint(INT_MAX, INT_MAX));

        template<class T>
        static QPoint findCenter(T items) {
            QPoint currentCenter;
            int count = 0;
            for (const auto &item : items) {
                count++;
                currentCenter += item->pos() + QPoint(item->size().width() / 2, item->size().height() / 2);
            }
            return currentCenter / count;
        }

        ItemGrid &grid() { return _grid; }

        const ItemGrid &grid() const { return _grid; }

        ItemCollection &items() { return _items; }

        const ItemCollection &items() const { return _items; }

        ItemCollection &selectedItems() { return _selectedItems; }

        const ItemCollection &selectedItems() const { return _selectedItems; }

        bool hasSelection() const { return !_selectedItems.sequence().empty(); }

        void selectAll();

        void deselectAll();

        void startDragging();

        void dragTo(QPoint delta);

        void finishDragging();

        void setDirty();

        void tryFlush();

    private:
        Grid<GridItem> _grid;
        ItemCollection _items;
        ItemCollection _selectedItems;
        bool _deferDirty;
        bool _isDirty = false;

        QPoint lastDragDelta;

        bool isAllDragAvailable(QPoint delta);

        QPoint findAvailableDelta(QPoint delta);

        void handleItemAdded(GridItem *const &item);
    };
}
