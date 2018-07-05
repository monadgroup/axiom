#include "Runtime.h"

#include "Frontend.h"

using namespace MaximCompiler;

Runtime::Runtime() : OwnedObject(MaximFrontend::maxim_create_runtime(), &MaximFrontend::maxim_destroy_runtime) {}

uint64_t Runtime::nextId() {
    return MaximFrontend::maxim_allocate_id(get());
}

void Runtime::runUpdate() {
    MaximFrontend::maxim_run_update(get());
}

void Runtime::commit(MaximCompiler::Transaction transaction) {
    MaximFrontend::maxim_commit(get(), transaction.release());
}
