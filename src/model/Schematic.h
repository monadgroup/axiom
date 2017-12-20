#pragma once
#include <memory>
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QString>

namespace AxiomModel {

    class Node;

    class Schematic {
        Q_OBJECT

    public:
        QVector<std::unique_ptr<Node>> nodes;

        virtual QString getName() = 0;

        int panX;
        int panY;
    };

}
