#include "Transaction.h"

#include "Frontend.h"

using namespace MaximCompiler;

Transaction::Transaction() : OwnedObject(MaximFrontend::maxim_create_transaction(), &MaximFrontend::maxim_destroy_transaction) {}

SurfaceRef Transaction::buildSurface(uint64_t id, const std::string &name) {
    return SurfaceRef(MaximFrontend::maxim_build_surface(get(), id, name.c_str()));
}
