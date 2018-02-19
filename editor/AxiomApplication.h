#pragma once

#include <QApplication>
#include "widgets/windows/MainWindow.h"

namespace MaximRuntime {
    class Runtime;
}

namespace AxiomModel {
    class Project;
}

class AxiomApplication : public QApplication {
Q_OBJECT

public:
    static AxiomApplication *main;
    static MaximRuntime::Runtime *runtime;
    static AxiomModel::Project *project;

    AxiomGui::MainWindow win;

    AxiomApplication(int argc, char *argv[]);
};
