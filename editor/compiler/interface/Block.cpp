#include "Block.h"

using namespace MaximCompiler;

Block::Block(void *handle) : OwnedObject(handle, &MaximFrontend::maxim_destroy_block) {}

std::variant<Block, Error> Block::compile(uint64_t id, const QString &name, const QString &code) {
    void *out = nullptr;
    if (MaximFrontend::maxim_compile_block(id, name.toUtf8().constData(), code.toUtf8().constData(), &out, &out)) {
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

Block Block::clone() const {
    return Block(MaximFrontend::maxim_block_clone(get()));
}
