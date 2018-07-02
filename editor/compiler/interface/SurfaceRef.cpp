#include "SurfaceRef.h"

#include "Frontend.h"

using namespace MaximCompiler;

SurfaceRef::SurfaceRef(void *handle) : handle(handle) {}

void SurfaceRef::addValueGroup(MaximCompiler::VarType vartype, MaximCompiler::ValueGroupSource source) {
    MaximFrontend::maxim_build_value_group(get(), vartype.release(), source.release());
}

NodeRef SurfaceRef::addCustomNode(uint64_t blockId) {
    return NodeRef(MaximFrontend::maxim_build_custom_node(get(), blockId));
}

NodeRef SurfaceRef::addGroupNode(uint64_t surfaceId) {
    return NodeRef(MaximFrontend::maxim_build_group_node(get(), surfaceId));
}
