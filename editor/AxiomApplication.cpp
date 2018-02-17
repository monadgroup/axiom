#include "AxiomApplication.h"

#include "util.h"
#include "editor/model/node/CustomNode.h"

#include "../compiler/runtime/Runtime.h"
#include "../compiler/runtime/Surface.h"
#include "../compiler/runtime/Node.h"
#include "../compiler/codegen/Operator.h"
#include "../compiler/codegen/Converter.h"
#include "../compiler/codegen/Function.h"

AxiomApplication *AxiomApplication::main = nullptr;
MaximRuntime::Runtime *AxiomApplication::runtime = nullptr;
AxiomModel::Project *AxiomApplication::project = nullptr;

AxiomApplication::AxiomApplication(int argc, char **argv) : QApplication(argc, argv) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));
}
