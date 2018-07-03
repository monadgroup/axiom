#pragma once

#include "widgets/windows/MainWindow.h"
#include <QApplication>

namespace AxiomModel {
    class Project;
}

class AxiomApplication : public QApplication {
    Q_OBJECT

public:
    static AxiomApplication main;

    AxiomApplication();
};
