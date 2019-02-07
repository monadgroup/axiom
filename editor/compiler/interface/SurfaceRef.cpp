#include "SurfaceRef.h"

#include "Frontend.h"

using namespace MaximCompiler;

SurfaceRef::SurfaceRef(void *handle) : handle(handle) {}

void SurfaceRef::addValueGroup(MaximCompiler::VarType vartype, MaximCompiler::ValueGroupSource source) {
    MaximFrontend::maxim_build_value_group(get(), vartype.release(), source.release());
}

NodeRef SurfaceRef::addCustomNode(uint64_t blockId, size_t controlInitializerCount, ControlInitializer *initializers) {
    std::vector<MaximFrontend::MaximControlInitializer *> initializerPtrs;
    initializerPtrs.reserve(controlInitializerCount);
    for (size_t i = 0; i < controlInitializerCount; i++) {
        initializerPtrs.push_back(initializers[i].release());
    }

    return NodeRef(
        MaximFrontend::maxim_build_custom_node(get(), blockId, controlInitializerCount, &initializerPtrs[0]));
}

NodeRef SurfaceRef::addGroupNode(uint64_t surfaceId) {
    return NodeRef(MaximFrontend::maxim_build_group_node(get(), surfaceId));
}
