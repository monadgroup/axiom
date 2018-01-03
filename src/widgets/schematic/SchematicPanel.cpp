#include "SchematicPanel.h"

#include "src/util.h"
#include "src/AxiomApplication.h"
#include "SchematicView.h"

using namespace AxiomGui;

SchematicPanel::SchematicPanel(AxiomModel::Schematic *schematic) : DockPanel(schematic->name()) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/SchematicPanel.qss"));

    auto view = new SchematicView(schematic);
    setWidget(view);
}
