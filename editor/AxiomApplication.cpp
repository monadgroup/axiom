#include "AxiomApplication.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

#include "util.h"
#include "editor/model/node/CustomNode.h"

AxiomApplication AxiomApplication::main;

int argc = 1;
char **argv = new char*[1];

AxiomApplication::AxiomApplication() : QApplication(argc, argv) {
    // initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));
}
