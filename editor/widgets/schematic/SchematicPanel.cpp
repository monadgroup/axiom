#include "SchematicPanel.h"

#include "editor/util.h"
#include "editor/AxiomApplication.h"
#include "SchematicView.h"
#include "editor/model/schematic/Schematic.h"

using namespace AxiomGui;

SchematicPanel::SchematicPanel(MainWindow *window, AxiomModel::Schematic *schematic)
    : DockPanel(schematic->name()), window(window) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/SchematicPanel.qss"));

    connect(schematic, &AxiomModel::Schematic::nameChanged,
            this, &SchematicPanel::setWindowTitle);
    connect(schematic, &AxiomModel::Schematic::cleanup,
            this, &SchematicPanel::close);

    setWidget(new SchematicView(this, schematic));
    widget()->setParent(this);
}

void SchematicPanel::closeEvent(QCloseEvent *event) {
    DockPanel::closeEvent(event);
    emit closed();
}
