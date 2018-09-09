#include "Runtime.h"

#include "Frontend.h"

using namespace MaximCompiler;

Runtime::Runtime(bool includeUi, bool minSize)
    : OwnedObject(MaximFrontend::maxim_create_runtime(includeUi, minSize), &MaximFrontend::maxim_destroy_runtime) {}

uint64_t Runtime::nextId() {
    return MaximFrontend::maxim_allocate_id(get());
}

void Runtime::runUpdate() {
    MaximFrontend::maxim_run_update(get());
}

void Runtime::setBpm(float bpm) {
    MaximFrontend::maxim_set_bpm(get(), bpm);
}

float Runtime::getBpm() {
    return MaximFrontend::maxim_get_bpm(get());
}

void Runtime::setSampleRate(float sampleRate) {
    MaximFrontend::maxim_set_sample_rate(get(), sampleRate);
}

float Runtime::getSampleRate() {
    return MaximFrontend::maxim_get_sample_rate(get());
}

void Runtime::commit(MaximCompiler::Transaction transaction) {
    MaximFrontend::maxim_commit(get(), transaction.release());
}

bool Runtime::isNodeExtracted(uint64_t surface, size_t node) {
    return MaximFrontend::maxim_is_node_extracted(get(), surface, node);
}

AxiomModel::NumValue Runtime::convertNum(AxiomModel::FormType targetForm, const AxiomModel::NumValue &value) {
    AxiomModel::NumValue result;
    MaximFrontend::maxim_convert_num(get(), &result, (uint8_t) targetForm, &value);
    return result;
}

void *Runtime::getPortalPtr(size_t portal) {
    return MaximFrontend::maxim_get_portal_ptr(get(), portal);
}

void *Runtime::getRootPtr() {
    return MaximFrontend::maxim_get_root_ptr(get());
}

void *Runtime::getNodePtr(uint64_t surface, void *surfacePtr, size_t node) {
    return MaximFrontend::maxim_get_node_ptr(get(), surface, surfacePtr, node);
}

void *Runtime::getSurfacePtr(void *nodePtr) {
    return MaximFrontend::maxim_get_surface_ptr(nodePtr);
}

void *Runtime::getBlockPtr(void *nodePtr) {
    return MaximFrontend::maxim_get_block_ptr(nodePtr);
}

MaximFrontend::ControlPointers Runtime::getControlPtrs(uint64_t block, void *blockPtr, size_t control) {
    return MaximFrontend::maxim_get_control_ptrs(get(), block, blockPtr, control);
}
