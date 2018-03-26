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

    signals:

        void closed();

    protected:

        void closeEvent(QCloseEvent *event) override;

    private:
        QGraphicsScene *scene;
    };

}
