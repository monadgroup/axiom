#include "SchematicPanel.h"

#include "src/util.h"
#include "SchematicCanvas.h"

using namespace AxiomGui;

SchematicPanel::SchematicPanel(QWidget *parent) : QDockWidget("todo", parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/SchematicPanel.qss"));

    auto canvas = new SchematicCanvas();
    setWidget(canvas);
}
