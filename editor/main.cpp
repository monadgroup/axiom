#include "compiler/runtime/Runtime.h"
#include "widgets/windows/MainWindow.h"
#include "model/Project.h"
#include "AxiomApplication.h"

int main(int argc, char *argv[]) {
    AxiomApplication main;

    MaximRuntime::Runtime runtime;
    AxiomModel::Project project(&runtime);
    AxiomGui::MainWindow window;
    window.loadProject(&project);

    window.show();
    return main.exec();
}
