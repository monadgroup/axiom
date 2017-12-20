#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>

namespace AxiomModel {

    class ISchematic;

    class Node {
        Q_OBJECT

    public:
        ISchematic *parent;

        QString name;

        int x;
        int y;
        int width;
        int height;

        bool isSelected;
    };

}
