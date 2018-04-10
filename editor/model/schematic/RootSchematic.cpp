#include "RootSchematic.h"

#include "../Project.h"
#include "../node/IONode.h"
#include "compiler/runtime/RootSurface.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

RootSchematic::RootSchematic(Project *project, MaximRuntime::RootSurface *runtime) : Schematic(project, runtime) {
    addItem(std::make_unique<IONode>(this, &runtime->input, "Input", QPoint(-3, 0)));
    addItem(std::make_unique<IONode>(this, &runtime->output, "Output", QPoint(3, 0)));

    runtime->scheduleGraphUpdate();
}

QString RootSchematic::name() {
    return "Root";
}
