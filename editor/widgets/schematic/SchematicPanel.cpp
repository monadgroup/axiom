#include "SchematicPanel.h"

#include "editor/util.h"
#include "editor/AxiomApplication.h"
#include "SchematicView.h"
#include "editor/model/schematic/Schematic.h"

using namespace AxiomGui;

SchematicPanel::SchematicPanel(MainWindow *window, AxiomModel::Schematic *schematic)
        : DockPanel(schematic->name()), window(window) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/SchematicPanel.qss"));

    connect(schematic, &AxiomModel::Schematic::removed,
            this, &SchematicPanel::remove);
    connect(schematic, &AxiomModel::Schematic::nameChanged,
            this, &SchematicPanel::setWindowTitle);

    auto view = new SchematicView(this, schematic);
    setWidget(view);
}

void SchematicPanel::remove() {
    window->removeDockWidget(this);
}
