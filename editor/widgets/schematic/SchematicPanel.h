#pragma once

#include <QtWidgets/QGraphicsScene>

#include "../dock/DockPanel.h"

namespace AxiomModel {
    class Schematic;
}

namespace AxiomGui {

    class MainWindow;

    class SchematicPanel : public DockPanel {
    Q_OBJECT

    public:
        MainWindow *window;

        explicit SchematicPanel(MainWindow *window, AxiomModel::Schematic *schematic);

    private slots:

        void remove();

    private:
        QGraphicsScene *scene;
    };

}
