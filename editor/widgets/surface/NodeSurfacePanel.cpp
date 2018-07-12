#include "NodeSurfacePanel.h"

#include "NodeSurfaceView.h"
#include "editor/AxiomApplication.h"
#include "editor/model/objects/NodeSurface.h"
#include "editor/util.h"

using namespace AxiomGui;

NodeSurfacePanel::NodeSurfacePanel(MainWindow *window, AxiomModel::NodeSurface *surface)
    : DockPanel(surface->name()), window(window) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/styles/SchematicPanel.qss"));

    surface->nameChanged.connect(this, &NodeSurfacePanel::setWindowTitle);
    surface->removed.connect(this, &NodeSurfacePanel::close);

    setWidget(new NodeSurfaceView(this, surface));
    widget()->setParent(this);
}

void NodeSurfacePanel::closeEvent(QCloseEvent *event) {
    DockPanel::closeEvent(event);
    emit closed();
}
