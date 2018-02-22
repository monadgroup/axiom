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
    static AxiomApplication main;

    AxiomApplication();

};
