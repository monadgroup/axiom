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
        bool selected() const { return m_selected; }

    public slots:
        void setName(const QString &name);
        void setPos(QPoint pos);
        void setSize(QSize size);
        void setSelected(bool selected);
        void remove();

    signals:
        void nameChanged(const QString &newName);
        void posChanged(QPoint newPos);
        void sizeChanged(QSize newSize);
        void selectedChanged(bool newSelected);
        void removed();

    private:
        QString m_name;
        QPoint m_pos;
        QSize m_size;
        bool m_selected;
    };

}
