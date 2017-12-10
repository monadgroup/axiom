#include <QApplication>
#include <QtCore/QFile>
#include "widgets/windows/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    auto stylesheet = new QFile(":/MainStyles.qss");
    stylesheet->open(QIODevice::ReadOnly | QIODevice::Text);
    app.setStyleSheet(QLatin1String(stylesheet->readAll()));

    AxiomGui::MainWindow win;
    win.show();
    return app.exec();
}
