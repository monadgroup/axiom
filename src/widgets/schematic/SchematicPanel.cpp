#include "SchematicPanel.h"

#include "src/util.h"
#include "src/AxiomApplication.h"
#include "SchematicCanvas.h"

using namespace AxiomGui;

SchematicPanel::SchematicPanel(QWidget *parent) : QDockWidget("todo", parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/SchematicPanel.qss"));

    auto canvas = new SchematicCanvas(&AxiomApplication::project->root);
    setWidget(canvas);
}
