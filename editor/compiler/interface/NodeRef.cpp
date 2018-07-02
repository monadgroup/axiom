#include "NodeRef.h"

#include "Frontend.h"

using namespace MaximCompiler;

NodeRef::NodeRef(void *handle) : handle(handle) {}

void NodeRef::addValueSocket(size_t groupId, bool valueWritten, bool valueRead, bool isExtractor) {
    MaximFrontend::maxim_build_value_socket(get(), groupId, valueWritten, valueRead, isExtractor);
}
