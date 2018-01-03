#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointF>

#include "GridItem.h"

namespace std {
    template<>
    struct hash<QPoint> {
        std::size_t operator()(const QPoint &p) const {
            return std::hash<int>()(p.x()) ^ (std::hash<int>()(p.y()) >> 1);
        }
    };
}

namespace AxiomModel {

    class GridSurface : public QObject {
        Q_OBJECT

    public:
        std::vector<std::unique_ptr<GridItem>> const &items() const { return m_items; }

        bool hasSelection() const { return !selectedItems.empty(); }

        void addItem(std::unique_ptr<GridItem> item);

        bool positionAvailable(QPoint pos, QSize size, const GridItem *ignore = nullptr) const;

        QPoint findNearestPos(QPoint pos, QSize size, const GridItem *ignore = nullptr) const;

        void freeGridRect(QPoint pos, QSize size);
        void setGridRect(QPoint pos, QSize size, const GridItem *item);

    public slots:
        void deleteSelectedItems();
        void selectAll();
        void deselectAll();

    private slots:
        void removeItem(GridItem *item);
        void selectItem(GridItem *item, bool exclusive);
        void deselectItem(GridItem *item);

        void startDragging();
        void dragTo(QPoint delta);
        void finishDragging();

    signals:
        void itemAdded(GridItem *item);

    private:
        std::vector<std::unique_ptr<GridItem>> m_items;
        std::vector<GridItem*> selectedItems;
        std::unordered_map<QPoint, const GridItem*> grid;

        QPoint lastDragDelta;

        bool isAllDragAvailable(QPoint delta);

        QPoint findAvailableDelta(QPoint delta);
    };

}
