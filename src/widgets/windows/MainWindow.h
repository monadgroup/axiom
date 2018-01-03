#pragma once

#include <QtWidgets/QMainWindow>

namespace AxiomGui {

    class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        MainWindow();

    public slots:

        void showAbout();
    };

}
