#include <QApplication>

#include "AxiomApplication.h"
#include "../compiler/runtime/Runtime.h"
#include "../compiler/runtime/Surface.h"
#include "../compiler/runtime/Node.h"
#include "../compiler/codegen/Operator.h"
#include "../compiler/codegen/Converter.h"
#include "../compiler/codegen/Function.h"

int main(int argc, char *argv[]) {
    //MaximRuntime::Runtime runtime;
    //runtime.rebuild();

    AxiomApplication::main = new AxiomApplication(argc, argv);
    AxiomApplication::main->win.show();
    return AxiomApplication::main->exec();
}
