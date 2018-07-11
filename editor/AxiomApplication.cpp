#include "AxiomApplication.h"

#include <QtCore/QtCore>

#include "compiler/interface/Frontend.h"
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
    MaximFrontend::maxim_initialize();
    AxiomGui::GlobalActions::setupActions();

    Q_INIT_RESOURCE(res);
    setStyleSheet(AxiomUtil::loadStylesheet(":/styles/MainStyles.qss"));
}
