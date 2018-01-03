#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#include "../GridItem.h"

namespace AxiomModel {

    class Schematic;

    class Node : public GridItem {
    Q_OBJECT

    public:
        explicit Node(Schematic *parent);

        QString name() const { return m_name; }

    public slots:
        void setName(const QString &name);

    signals:
        void nameChanged(const QString &newName);

    private:
        QString m_name = "";
    };

}
