#include "AxiomApplication.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <QtCore/QtCore>

#include "util.h"
#include "editor/model/node/CustomNode.h"
#include "widgets/GlobalActions.h"

#ifdef Q_OS_WIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif Q_OS_UNIX
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif Q_OS_DARWIN
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif

AxiomApplication AxiomApplication::main;

int argc = 1;
char **argv = new char *[1];

AxiomApplication::AxiomApplication() : QApplication(argc, argv) {
    // initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    AxiomGui::GlobalActions::setupActions();

    setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));
}
