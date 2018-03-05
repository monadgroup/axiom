#include "RootSchematic.h"

#include "../node/OutputNode.h"
#include "compiler/runtime/RootSurface.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

RootSchematic::RootSchematic(MaximRuntime::RootSurface *runtime) : Schematic(runtime) {
    addItem(std::make_unique<OutputNode>(this, &runtime->output, QPoint(0, 0)));

    runtime->scheduleGraphUpdate();
    runtime->runtime()->compile();
}

QString RootSchematic::name() {
    return "Root";
}
