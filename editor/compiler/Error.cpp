#include "Error.h"

using namespace MaximCompiler;

Error::Error(void *handle) : OwnedObject(handle, &MaximFrontend::maxim_destroy_error) {}

std::string Error::getDescription() const {
    auto cStr = MaximFrontend::maxim_error_get_description(get());
    std::string resultStr(cStr);
    MaximFrontend::maxim_destroy_string(cStr);
    return resultStr;
}

MaximFrontend::SourceRange Error::getRange() const {
    return MaximFrontend::maxim_error_get_range(get());
}
