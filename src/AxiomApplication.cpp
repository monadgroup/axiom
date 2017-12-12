#include "AxiomApplication.h"

#include "util.h"

AxiomApplication *AxiomApplication::main = nullptr;

AxiomApplication::AxiomApplication(int argc, char **argv) : QApplication(argc, argv) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));
}
