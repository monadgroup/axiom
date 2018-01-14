#pragma once

#include <QtWidgets/QGraphicsScene>

#include "../dock/DockPanel.h"

namespace AxiomModel {
    class Schematic;
}

namespace AxiomGui {

    class SchematicPanel : public DockPanel {
    Q_OBJECT

    public:
        explicit SchematicPanel(AxiomModel::Schematic *schematic);

    private:
        QGraphicsScene *scene;
    };

}
