#include "AxiomApplication.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <QtCore/QtCore>

#include "util.h"
#include "widgets/GlobalActions.h"

#ifdef AXIOM_STATIC_BUILD
#ifdef Q_OS_WIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
/*
 * TODO/NOTE: is this precise enough, or should a distinction between
 * Linux/${foo}BSD/... be made?
 * xcb should normally work on anything unix-like, though.
 */
#elif defined(Q_OS_UNIX)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(Q_OS_DARWIN)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif
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
