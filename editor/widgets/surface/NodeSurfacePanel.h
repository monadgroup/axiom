#pragma once

#include <QtWidgets/QGraphicsScene>

#include "../dock/DockWidget.h"
#include "common/TrackedObject.h"

namespace AxiomModel {
    class NodeSurface;
}

namespace AxiomGui {

    class MainWindow;

    class NodeSurfacePanel : public ads::CDockWidget, public AxiomCommon::TrackedObject {
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
