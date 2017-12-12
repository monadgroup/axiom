#include <QApplication>

#include "AxiomApplication.h"

int main(int argc, char *argv[]) {
    AxiomApplication::main = new AxiomApplication(argc, argv);
    AxiomApplication::main->win.show();
    return AxiomApplication::main->exec();
}
