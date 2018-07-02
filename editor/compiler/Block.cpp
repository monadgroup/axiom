#include "Block.h"

#include "Frontend.h"

using namespace MaximCompiler;

Block::Block(void *handle) : OwnedObject(handle, &MaximFrontend::maxim_destroy_block) {}

std::variant<Block, Error> Block::compile(uint64_t id, const std::string &name, const std::string &code) {
    void *out = nullptr;
    if (MaximFrontend::maxim_compile_block(id, name.c_str(), code.c_str(), out, out)) {
        return Block(out);
    } else {
        return Error(out);
    }
}

size_t Block::controlCount() const {
    return MaximFrontend::maxim_block_get_control_count(get());
}

ControlRef Block::getControl(size_t index) const {
    return ControlRef(MaximFrontend::maxim_block_get_control(get(), index));
}
