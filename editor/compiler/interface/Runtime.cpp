#include "Runtime.h"

#include "Frontend.h"
#include "Jit.h"

using namespace MaximCompiler;

Runtime::Runtime(bool includeUi, bool minSize, Jit *jit)
    : OwnedObject(MaximFrontend::maxim_create_runtime(includeUi, minSize, jit->get()),
                  &MaximFrontend::maxim_destroy_runtime) {}

uint64_t Runtime::nextId() {
    return MaximFrontend::maxim_allocate_id(get());
}

void Runtime::runUpdate() {
    MaximFrontend::maxim_run_update(get());
}

void Runtime::commit(MaximCompiler::Transaction transaction) {
    MaximFrontend::maxim_commit(get(), transaction.release());
}
