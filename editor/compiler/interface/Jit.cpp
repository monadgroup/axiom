#include "Jit.h"

#include "Frontend.h"

using namespace MaximCompiler;

Jit::Jit() : OwnedObject(MaximFrontend::maxim_create_jit(), &MaximFrontend::maxim_destroy_jit) {}
