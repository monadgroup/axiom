#include "RootSchematic.h"

#include "../node/OutputNode.h"
#include "compiler/runtime/RootSchematic.h"

using namespace AxiomModel;

RootSchematic::RootSchematic(MaximRuntime::RootSchematic *runtime) : Schematic(runtime) {
    addItem(std::make_unique<OutputNode>(this, QPoint(0, 0)));
}

QString RootSchematic::name() {
    return "Root";
}
