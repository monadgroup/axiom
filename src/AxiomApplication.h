#pragma once
#include <QApplication>
#include "widgets/windows/MainWindow.h"

class AxiomApplication : public QApplication {
    Q_OBJECT

public:
    static AxiomApplication *main;

    AxiomGui::MainWindow win;

    AxiomApplication(int argc, char *argv[]);
};
