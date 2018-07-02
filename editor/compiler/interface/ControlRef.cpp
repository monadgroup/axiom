#include "ControlRef.h"

#include "Frontend.h"

using namespace MaximCompiler;

ControlRef::ControlRef(void *handle) : handle(handle) {}

std::string ControlRef::getName() const {
    auto cStr = MaximFrontend::maxim_control_get_name(get());
    std::string resultStr(cStr);
    MaximFrontend::maxim_destroy_string(cStr);
    return resultStr;
}

ControlType ControlRef::getType() const {
    return (ControlType) MaximFrontend::maxim_control_get_type(get());
}

bool ControlRef::getIsWritten() const {
    return MaximFrontend::maxim_control_get_written(get());
}

bool ControlRef::getIsRead() const {
    return MaximFrontend::maxim_control_get_read(get());
}
