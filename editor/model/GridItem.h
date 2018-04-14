#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <memory>

namespace AxiomModel {

    class GridSurface;

    class GridItem : public QObject {
    Q_OBJECT

    public:
        GridSurface *parentSurface;

        GridItem(GridSurface *parent, QPoint pos, QSize size);

        QPoint pos() const { return m_pos; }

        QSize size() const { return m_size; }

        bool isSelected() const { return m_selected; }

        bool isDragAvailable(QPoint delta);

        virtual bool isMovable() const = 0;

        virtual bool isResizable() const = 0;

        virtual bool isDeletable() const = 0;

        virtual std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const = 0;

    public slots:

        virtual void doRuntimeUpdate() {}

        virtual void saveValue() {}

        virtual void restoreValue() {}

        void setPos(QPoint pos, bool updateGrid = true, bool checkPositions = true);

        void setSize(QSize size);

        virtual void setCorners(QPoint topLeft, QPoint bottomRight);

        void select(bool exclusive);

        void deselect();

        virtual void remove();

        virtual void startDragging();

        void dragTo(QPoint delta);

        virtual void finishDragging();

        virtual void serialize(QDataStream &stream, QPoint offset) const;

        virtual void deserialize(QDataStream &stream, QPoint offset);

    signals:

        void beforePosChanged(QPoint newPos);

        void posChanged(QPoint newPos);

        void beforeSizeChanged(QSize newSize);

        void sizeChanged(QSize newSize);

        void selectedChanged(bool selected);

        void selected(bool exclusive);

        void deselected();

        void startedDragging();

        void draggedTo(QPoint delta);

        void finishedDragging();

        void removed();

        void cleanup();

    private:
        QPoint m_pos = QPoint(0, 0);
        QSize m_size = QSize(0, 0);
        bool m_selected = false;

        QPoint dragStartPos;
    };

}
