#include "NodeSurfacePanel.h"

#include "NodeSurfaceView.h"
#include "editor/AxiomApplication.h"
#include "editor/model/objects/NodeSurface.h"
#include "editor/util.h"

using namespace AxiomGui;

NodeSurfacePanel::NodeSurfacePanel(MainWindow *window, AxiomModel::NodeSurface *surface)
    : ads::CDockWidget(surface->name()), window(window) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/styles/SchematicPanel.qss"));

    surface->nameChanged.connect(this, &NodeSurfacePanel::setWindowTitle);
    surface->removed.connect(this, &NodeSurfacePanel::cleanup);

    setWidget(new NodeSurfaceView(this, surface));
    widget()->setParent(this);
}

void NodeSurfacePanel::cleanup() {
    toggleView(false);
}
