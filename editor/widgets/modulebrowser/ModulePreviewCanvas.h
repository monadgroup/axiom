#pragma once

#include <QtWidgets/QGraphicsScene>

namespace AxiomModel {
    class Schematic;

    class Node;

    class ConnectionWire;
}

namespace AxiomGui {

    class ModulePreviewCanvas : public QGraphicsScene {
    Q_OBJECT

    public:

        explicit ModulePreviewCanvas(const AxiomModel::Schematic *schematic);

    signals:

        void contentChanged();

    private slots:

        void addNode(AxiomModel::Node *node);

        void addWire(AxiomModel::ConnectionWire *wire);

    };

}
