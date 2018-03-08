#include "RootSchematic.h"

#include "editor/model/node/IONode.h"
#include "compiler/runtime/RootSurface.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

RootSchematic::RootSchematic(MaximRuntime::RootSurface *runtime) : Schematic(runtime) {
    addItem(std::make_unique<IONode>(this, &runtime->input, "Input", QPoint(-3, 0)));
    addItem(std::make_unique<IONode>(this, &runtime->output, "Output", QPoint(3, 0)));

    runtime->scheduleGraphUpdate();
    runtime->runtime()->compile();
}

QString RootSchematic::name() {
    return "Root";
}
