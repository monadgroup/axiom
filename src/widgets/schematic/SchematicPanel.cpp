#include "SchematicPanel.h"

#include "src/util.h"
#include "src/AxiomApplication.h"
#include "SchematicView.h"

using namespace AxiomGui;

SchematicPanel::SchematicPanel(QWidget *parent) : QDockWidget("todo", parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/SchematicPanel.qss"));

    auto view = new SchematicView(&AxiomApplication::project->root);
    setWidget(view);
}
