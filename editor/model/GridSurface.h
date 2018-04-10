#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointF>

#include "Grid.h"
#include "GridItem.h"

namespace AxiomModel {

    class GridSurface : public QObject {
    Q_OBJECT

    public:
        Grid<GridItem> grid;

        explicit GridSurface(QPoint minRect = QPoint(INT_MIN, INT_MIN), QPoint maxRect = QPoint(INT_MAX, INT_MAX));

        std::vector<std::unique_ptr<GridItem>> const &items() const { return m_items; }

        bool hasSelection() const { return !m_selectedItems.empty(); }

        std::vector<GridItem *> const &selectedItems() const { return m_selectedItems; }

        void addItem(std::unique_ptr<GridItem> item);

        void insertItem(size_t index, std::unique_ptr<GridItem> item);

        void cloneTo(GridSurface *surface) const;

    public slots:

        virtual void doRuntimeUpdate();

        virtual void saveValue();

        virtual void restoreValue();

        void deleteAll();

        void deleteSelectedItems();

        void selectAll();

        void deselectAll();

        void startDragging();

        void dragTo(QPoint delta);

        void finishDragging();

        void flushGrid();

    private slots:

        void removeItem(GridItem *item);

        void selectItem(GridItem *item, bool exclusive);

        void deselectItem(GridItem *item);

    signals:

        void itemAdded(GridItem *item);

        void hasSelectionChanged(bool hasSelection);

        void gridChanged();

    private:
        std::vector<std::unique_ptr<GridItem>> m_items;
        std::vector<GridItem *> m_selectedItems;

        QPoint lastDragDelta;

        bool isAllDragAvailable(QPoint delta);

        QPoint findAvailableDelta(QPoint delta);
    };

}
