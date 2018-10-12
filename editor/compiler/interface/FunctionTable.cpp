#include "FunctionTable.h"

#include "Frontend.h"

using namespace MaximCompiler;

size_t FunctionTable::size() {
    return MaximFrontend::maxim_get_function_table_size();
}

QString FunctionTable::find(size_t index) {
    auto cStr = MaximFrontend::maxim_get_function_table_entry(index);
    auto resultStr = QString::fromUtf8(cStr);
    MaximFrontend::maxim_destroy_string(cStr);
    return resultStr;
}
