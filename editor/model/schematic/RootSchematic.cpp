#include "RootSchematic.h"

#include "../node/OutputNode.h"
#include "compiler/runtime/RootSchematic.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

RootSchematic::RootSchematic(MaximRuntime::RootSchematic *runtime) : Schematic(runtime) {
    addItem(std::make_unique<OutputNode>(this, &runtime->output, QPoint(0, 0)));

    runtime->scheduleCompile();
    runtime->runtime()->compileAndDeploy();
}

QString RootSchematic::name() {
    return "Root";
}
