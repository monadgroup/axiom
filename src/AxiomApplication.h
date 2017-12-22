#pragma once
#include <QApplication>
#include "widgets/windows/MainWindow.h"
#include "src/model/Project.h"

class AxiomApplication : public QApplication {
Q_OBJECT

public:
    static AxiomApplication *main;
    static AxiomModel::Project *project;

    AxiomGui::MainWindow win;

    AxiomApplication(int argc, char *argv[]);
};
