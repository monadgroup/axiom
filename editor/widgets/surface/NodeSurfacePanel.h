#pragma once

#include <QtWidgets/QGraphicsScene>

#include "../dock/DockPanel.h"
#include "editor/model/Hookable.h"

namespace AxiomModel {
    class NodeSurface;
}

namespace AxiomGui {

    class MainWindow;

    class NodeSurfacePanel : public DockPanel, public AxiomModel::Hookable {
    Q_OBJECT

    public:
        MainWindow *window;

        explicit NodeSurfacePanel(MainWindow *window, AxiomModel::NodeSurface *surface);

    signals:

        void closed();

    protected:

        void closeEvent(QCloseEvent *event) override;

    private:
        QGraphicsScene *scene;
    };

}
