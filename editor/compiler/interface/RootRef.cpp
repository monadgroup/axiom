#include "RootRef.h"

#include "Frontend.h"

using namespace MaximCompiler;

RootRef::RootRef(void *handle) : handle(handle) {}

void RootRef::addSocket(MaximCompiler::VarType vartype) {
    MaximFrontend::maxim_build_root_socket(get(), vartype.release());
}
