#include "VarType.h"

#include "Frontend.h"

using namespace MaximCompiler;

VarType::VarType(void *handle) : OwnedObject(handle, &MaximFrontend::maxim_destroy_vartype) {}

VarType VarType::num() {
    return VarType(MaximFrontend::maxim_vartype_num());
}

VarType VarType::midi() {
    return VarType(MaximFrontend::maxim_vartype_midi());
}

VarType VarType::tuple(MaximCompiler::VarType *types, size_t count) {
    std::vector<void *> mapped_types;
    mapped_types.reserve(count);
    for (size_t i = 0; i < count; i++) {
        mapped_types[i] = types[i].release();
    }
    return VarType(MaximFrontend::maxim_vartype_tuple(&mapped_types[0], count));
}

VarType VarType::array(MaximCompiler::VarType subtype) {
    return VarType(MaximFrontend::maxim_vartype_array(subtype.release()));
}

VarType VarType::ofControl(ControlType controlType) {
    return VarType(MaximFrontend::maxim_vartype_of_control((uint8_t) controlType));
}

VarType VarType::clone() {
    return VarType(MaximFrontend::maxim_vartype_clone(get()));
}
