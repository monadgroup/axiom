#include "RootSurface.h"

#include "GeneratableModuleClass.h"
#include "ControlGroup.h"

using namespace MaximRuntime;

RootSurface::RootSurface(Runtime *runtime) : Surface(runtime, 0), output(this) {

}

void *RootSurface::getValuePtr(void *parentCtx) {
    return parentCtx;
}
