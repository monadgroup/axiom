#pragma once

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QRect>
#include <memory>

#include "common/Event.h"

namespace AxiomModel {

    class GridSurface;

    class GridItem {
    public:
        GridSurface *parentSurface;
        AxiomCommon::Event<QPoint> beforePosChanged;
        AxiomCommon::Event<QPoint> posChanged;
        AxiomCommon::Event<QSize> beforeSizeChanged;
        AxiomCommon::Event<QSize> sizeChanged;
        AxiomCommon::Event<bool> selectedChanged;
        AxiomCommon::Event<bool> selected;
        AxiomCommon::Event<> deselected;
        AxiomCommon::Event<> startedDragging;
        AxiomCommon::Event<QPoint> draggedTo;
        AxiomCommon::Event<> finishedDragging;

        GridItem(GridSurface *parent, QPoint pos, QSize size, bool selected);

        virtual ~GridItem();

        static void deserialize(QDataStream &stream, QPoint &pos, QSize &size, bool &selected);

        void serialize(QDataStream &stream) const;

        QPoint pos() const { return m_pos; }

        QSize size() const { return m_size; }

        QRect rect() const { return QRect(m_pos, m_size); }

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

        void setRect(QRect rect);

        virtual void setCorners(QPoint topLeft, QPoint bottomRight);

        void select(bool exclusive);

        void deselect();

        virtual void startDragging();

        void dragTo(QPoint delta);

        virtual void finishDragging();

        const QPoint &dragStartPos() const { return _dragStartPos; }

    private:
        QPoint m_pos;
        QSize m_size;
        bool m_selected;

        QPoint _dragStartPos;
    };

}
