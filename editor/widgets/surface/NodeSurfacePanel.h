#pragma once

#include <QtWidgets/QGraphicsScene>

#include "common/TrackedObject.h"
#include "vendor/dock/DockWidget.h"

namespace AxiomModel {
    class NodeSurface;
}

namespace AxiomGui {

    class MainWindow;

    class NodeSurfacePanel : public ads::CDockWidget, public AxiomCommon::TrackedObject {
        Q_OBJECT

    public:
        MainWindow *window;

        NodeSurfacePanel(MainWindow *window, AxiomModel::NodeSurface *surface);

    private slots:

        void cleanup();
    };
}
