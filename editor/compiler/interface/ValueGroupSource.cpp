#include "ValueGroupSource.h"

#include "Frontend.h"

using namespace MaximCompiler;

ValueGroupSource::ValueGroupSource(void *handle) : OwnedObject(handle, &MaximFrontend::maxim_destroy_valuegroupsource) {}

ValueGroupSource ValueGroupSource::none() {
    return ValueGroupSource(MaximFrontend::maxim_valuegroupsource_none());
}

ValueGroupSource ValueGroupSource::socket(size_t index) {
    return ValueGroupSource(MaximFrontend::maxim_valuegroupsource_socket(index));
}

ValueGroupSource ValueGroupSource::default_val(MaximCompiler::ConstantValue value) {
    return ValueGroupSource(MaximFrontend::maxim_valuegroupsource_default(value.release()));
}

ValueGroupSource ValueGroupSource::clone() {
    return ValueGroupSource(MaximFrontend::maxim_valuegroupsource_clone(get()));
}
