#include "AxiomApplication.h"

#include <QtCore/QDir>
#include <QtCore/QtCore>
#include <iostream>
#include <math.h>

#include "compiler/interface/Frontend.h"
#include "editor/resources/resource.h"
#include "util.h"
#include "widgets/GlobalActions.h"

#ifdef AXIOM_STATIC_BUILD
#ifdef Q_OS_WIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif defined(Q_OS_DARWIN)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#elif defined(Q_OS_UNIX)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#endif
#endif

int argc = 0;
char **argv = new char *[1];

AxiomApplication::AxiomApplication() : QApplication(argc, argv) {
    setApplicationName("Axiom");
    setApplicationVersion(AXIOM_VERSION);

    MaximFrontend::maxim_initialize();
    AxiomGui::GlobalActions::setupActions();

    Q_INIT_RESOURCE(res);
    setStyleSheet(AxiomUtil::loadStylesheet(":/styles/MainStyles.qss"));

    auto dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    std::cout << "Using data path " << dataPath.toStdString() << std::endl;

    // ensure the data path exists
    QDir().mkpath(dataPath);
}
