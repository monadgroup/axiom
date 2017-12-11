#include "NodeViewPanel.h"

#include "src/util.h"
#include "NodeViewCanvas.h"

using namespace AxiomGui;

NodeViewPanel::NodeViewPanel(QWidget *parent) : QDockWidget("todo", parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/NodeViewPanel.qss"));

    auto canvas = new NodeViewCanvas();
    setWidget(canvas);
}
