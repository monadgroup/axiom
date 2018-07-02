#include "OwnedObject.h"

#include <utility>

using namespace MaximCompiler;

OwnedObject::OwnedObject(void *handle, void (*destroy)(void *)) : handle(handle), destroy(destroy) {}

OwnedObject::~OwnedObject() {
    if (handle) destroy(handle);
}

OwnedObject::OwnedObject(OwnedObject &&other) noexcept : handle(other.handle) {
    other.handle = nullptr;
}

OwnedObject& OwnedObject::operator=(OwnedObject &&other) noexcept {
    if (handle) destroy(handle);
    handle = other.handle;
    other.handle = nullptr;
    return *this;
}

void* OwnedObject::release() {
    auto result = handle;
    handle = nullptr;
    return result;
}
