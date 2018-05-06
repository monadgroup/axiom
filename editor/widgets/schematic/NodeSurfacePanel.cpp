#include "NodeSurfacePanel.h"

#include "editor/util.h"
#include "editor/AxiomApplication.h"
#include "NodeSurfaceView.h"
#include "editor/model/objects/NodeSurface.h"

using namespace AxiomGui;

NodeSurfacePanel::NodeSurfacePanel(MainWindow *window, AxiomModel::NodeSurface *surface)
    : DockPanel(surface->name()), window(window) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/SchematicPanel.qss"));

    surface->nameChanged.listen(this, &NodeSurfacePanel::setWindowTitle);
    surface->removed.listen(this, &NodeSurfacePanel::close);

    setWidget(new NodeSurfaceView(this, surface));
    widget()->setParent(this);
}

void NodeSurfacePanel::closeEvent(QCloseEvent *event) {
    DockPanel::closeEvent(event);
    emit closed();
}
