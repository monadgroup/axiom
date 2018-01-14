#include "AxiomApplication.h"

#include "util.h"
#include "editor/model/node/CustomNode.h"

AxiomApplication *AxiomApplication::main = nullptr;
AxiomModel::Project *AxiomApplication::project = new AxiomModel::Project();

AxiomApplication::AxiomApplication(int argc, char **argv) : QApplication(argc, argv) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));
}
