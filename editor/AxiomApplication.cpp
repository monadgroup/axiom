#include "AxiomApplication.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

#include "util.h"
#include "editor/model/node/CustomNode.h"

AxiomApplication *AxiomApplication::main = nullptr;

AxiomApplication::AxiomApplication(int argc, char **argv) : QApplication(argc, argv) {
    // initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));
}
