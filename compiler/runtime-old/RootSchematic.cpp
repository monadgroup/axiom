#include "RootSchematic.h"

using namespace MaximRuntime;

RootSchematic::RootSchematic(Runtime *runtime) : Schematic(runtime, nullptr, 0), output(this) {

}

void *RootSchematic::getValuePtr(void *parentCtx) {
    return parentCtx;
}
