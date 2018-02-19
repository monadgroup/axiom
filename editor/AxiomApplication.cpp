#include "AxiomApplication.h"

#include "util.h"
#include "editor/model/node/CustomNode.h"

AxiomApplication *AxiomApplication::main = nullptr;
MaximRuntime::Runtime *AxiomApplication::runtime = nullptr;
AxiomModel::Project *AxiomApplication::project = nullptr;

AxiomApplication::AxiomApplication(int argc, char **argv) : QApplication(argc, argv) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));
}
