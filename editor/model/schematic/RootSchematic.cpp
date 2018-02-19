#include "RootSchematic.h"

using namespace AxiomModel;

RootSchematic::RootSchematic(MaximRuntime::Schematic *runtime) : Schematic(runtime) {

}

QString RootSchematic::name() {
    return "Root";
}
