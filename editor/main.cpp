#include <QApplication>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

#include "AxiomApplication.h"
#include "model/Project.h"
#include "../compiler/runtime/Runtime.h"

int main(int argc, char *argv[]) {
    // initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // initialize runtime (generates and loads core modules)
    AxiomApplication::runtime = new MaximRuntime::Runtime();
    AxiomApplication::project = new AxiomModel::Project(AxiomApplication::runtime);

    // show the window
    AxiomApplication::main = new AxiomApplication(argc, argv);
    AxiomApplication::main->win.show();
    return AxiomApplication::main->exec();
}
