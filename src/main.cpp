#include <QApplication>
#include "util.h"
#include "widgets/windows/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));

    AxiomGui::MainWindow win;
    win.show();
    return app.exec();
}
