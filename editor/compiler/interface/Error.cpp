#include "Error.h"

using namespace MaximCompiler;

Error::Error() : OwnedObject(nullptr, &MaximFrontend::maxim_destroy_error) {}

Error::Error(void *handle) : OwnedObject(handle, &MaximFrontend::maxim_destroy_error) {}

QString Error::getDescription() const {
    auto cStr = MaximFrontend::maxim_error_get_description(get());
    auto resultStr = QString::fromUtf8(cStr);
    MaximFrontend::maxim_destroy_string(cStr);
    return resultStr;
}

MaximFrontend::SourceRange Error::getRange() const {
    return MaximFrontend::maxim_error_get_range(get());
}
