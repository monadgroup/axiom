#pragma once

#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGraphicsScene>

namespace AxiomModel {
    class Schematic;
}

namespace AxiomGui {

    class SchematicPanel : public QDockWidget {
    Q_OBJECT

    public:
        explicit SchematicPanel(AxiomModel::Schematic *schematic);

    private:
        QGraphicsScene *scene;
    };

}
