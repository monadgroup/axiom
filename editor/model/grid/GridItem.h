#pragma once

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <memory>

#include "../Event.h"

namespace AxiomModel {

    class GridSurface;

    class GridItem {
    public:
        GridSurface *parentSurface;
        Event<QPoint> beforePosChanged;
        Event<QPoint> posChanged;
        Event<QSize> beforeSizeChanged;
        Event<QSize> sizeChanged;
        Event<bool> selectedChanged;
        Event<bool> selected;
        Event<> deselected;
        Event<> startedDragging;
        Event<QPoint> draggedTo;
        Event<> finishedDragging;

        GridItem(GridSurface *parent, QPoint pos, QSize size, bool selected);

        static void deserialize(QDataStream &stream, QPoint &pos, QSize &size, bool &selected);

        void serialize(QDataStream &stream) const;

        QPoint pos() const { return m_pos; }

        QSize size() const { return m_size; }

        bool isSelected() const { return m_selected; }

        bool isDragAvailable(QPoint delta);

        virtual bool isMovable() const = 0;

        virtual bool isResizable() const = 0;

        virtual bool isCopyable() const = 0;

        virtual bool isDeletable() const = 0;

        virtual void doRuntimeUpdate() {}

        virtual void saveValue() {}

        virtual void restoreValue() {}

        void setPos(QPoint pos, bool updateGrid = true, bool checkPositions = true);

        void setSize(QSize size);

        virtual void setCorners(QPoint topLeft, QPoint bottomRight);

        void select(bool exclusive);

        void deselect();

        virtual void startDragging();

        void dragTo(QPoint delta);

        virtual void finishDragging();

    private:
        QPoint m_pos;
        QSize m_size;
        bool m_selected;

        QPoint dragStartPos;
    };

}
