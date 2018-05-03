#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <QtCore/QString>
#include <QtCore/QPointF>

#include "Grid.h"
#include "../Pool.h"
#include "../Hookable.h"
#include "../Event.h"

namespace AxiomModel {

    class GridItem;

    class GridSurface : public Hookable {
    public:
        using ItemGrid = Grid<GridItem>;
        using ItemCollection = CollectionView<GridItem*>;
        
        Event<GridItem*> itemAdded;
        Event<bool> hasSelectionChanged;
        Event<> gridChanged;

        explicit GridSurface(ItemCollection view, QPoint minRect = QPoint(INT_MIN, INT_MIN), QPoint maxRect = QPoint(INT_MAX, INT_MAX));

        template<class T>
        static QPoint findCenter(const T &items) {
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

        void doRuntimeUpdate();

        void saveValue();

        void restoreValue();

        void deleteAll();

        void deleteSelectedItems();

        void selectAll();

        void deselectAll();

        void startDragging();

        void dragTo(QPoint delta);

        void finishDragging();

        void flushGrid();

    private:
        Grid<GridItem> _grid;
        ItemCollection _items;
        ItemCollection _selectedItems;

        QPoint lastDragDelta;

        static std::optional<GridItem*> filterSelected(GridItem *const &item);

        bool isAllDragAvailable(QPoint delta);

        QPoint findAvailableDelta(QPoint delta);

        void handleItemAdded(GridItem *const &item);

    };

}
