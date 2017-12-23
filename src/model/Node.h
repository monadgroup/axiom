#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QSize>

namespace AxiomModel {

    class Schematic;

    class Node : public QObject {
        Q_OBJECT

    public:
        Schematic *parent;

        explicit Node(Schematic *parent);

        QString name() const { return m_name; }
        QPoint pos() const { return m_pos; }
        QSize size() const { return m_size; }
        bool isSelected() const { return m_selected; }

    public slots:
        void setName(const QString &name);
        void setPos(QPoint pos);
        void setSize(QSize size);
        void select(bool exclusive);
        void deselect();
        void remove();

        void startDragging();
        void dragTo(QPoint delta);
        void finishDragging();

    signals:
        void nameChanged(const QString &newName);
        void beforePosChanged(QPoint newPos);
        void posChanged(QPoint newPos);
        void beforeSizeChanged(QSize newSize);
        void sizeChanged(QSize newSize);
        void selected(bool exclusive);
        void deselected();
        void startedDragging();
        void draggedTo(QPoint delta);
        void finishedDragging();
        void removed();

    private:
        QString m_name;
        QPoint m_pos;
        QSize m_size;
        bool m_selected;

        QPoint dragStartPos;
    };

}
