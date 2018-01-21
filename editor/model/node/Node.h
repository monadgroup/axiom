#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#include "../GridItem.h"
#include "NodeSurface.h"

namespace AxiomModel {

    class Schematic;

    class Node : public GridItem {
    Q_OBJECT

    public:
        enum class Type {
            CUSTOM,
            GROUP
        };
        Type type;

        NodeSurface surface;
        Schematic *parentSchematic;

        Node(Schematic *parent, QString name, Type type, QPoint pos, QSize size);

        QString name() const { return m_name; }

        bool isMovable() const override { return true; }

        bool isResizable() const override { return true; }

    public slots:

        void setName(const QString &name);

        void setCorners(QPoint topLeft, QPoint bottomRight) override;

    signals:

        void nameChanged(const QString &newName);

    private:
        QString m_name = "";
    };

}
