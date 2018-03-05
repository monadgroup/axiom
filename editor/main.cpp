#include "compiler/runtime/Runtime.h"
#include "widgets/windows/MainWindow.h"
#include "model/Project.h"
#include "AxiomApplication.h"
#include "compiler/runtime/GeneratableModuleClass.h"
#include "compiler/runtime/ControlGroup.h"
#include "compiler/codegen/Operator.h"
#include "compiler/codegen/Converter.h"
#include "compiler/codegen/Control.h"
#include "compiler/codegen/Function.h"

int main(int argc, char *argv[]) {
    MaximRuntime::Runtime runtime;
    AxiomModel::Project project(&runtime);
    AxiomGui::MainWindow window;
    window.loadProject(&project);

    window.show();
    return AxiomApplication::main.exec();
}
