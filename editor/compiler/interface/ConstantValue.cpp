#include "ConstantValue.h"

#include "Frontend.h"

using namespace MaximCompiler;

ConstantValue::ConstantValue(void *handle) : OwnedObject(handle, &MaximFrontend::maxim_destroy_constant) {}

ConstantValue ConstantValue::num(AxiomModel::NumValue value) {
    return ConstantValue(MaximFrontend::maxim_constant_num(value.left, value.right, (uint8_t) value.form));
}

ConstantValue ConstantValue::tuple(MaximCompiler::ConstantValue *items, size_t count) {
    std::vector<void *> mapped_values;
    mapped_values.reserve(count);
    for (size_t i = 0; i < count; i++) {
        mapped_values[i] = items[i].release();
    }
    return ConstantValue(MaximFrontend::maxim_constant_tuple(&mapped_values[0], count));
}

ConstantValue ConstantValue::clone() {
    return ConstantValue(MaximFrontend::maxim_constant_clone(get()));
}
