#include "Transaction.h"

#include "Frontend.h"

using namespace MaximCompiler;

Transaction::Transaction()
    : OwnedObject(MaximFrontend::maxim_create_transaction(), &MaximFrontend::maxim_destroy_transaction) {}

SurfaceRef Transaction::buildSurface(uint64_t id, const QString &name) {
    return SurfaceRef(MaximFrontend::maxim_build_surface(get(), id, name.toUtf8().constData()));
}

void Transaction::buildBlock(MaximCompiler::Block block) {
    MaximFrontend::maxim_build_block(get(), block.release());
}

void Transaction::printToStdout() const {
    MaximFrontend::maxim_print_transaction_to_stdout(get());
}
