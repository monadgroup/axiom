#include "Block.h"

using namespace MaximCompiler;

Block::Block() : OwnedObject(nullptr, &MaximFrontend::maxim_destroy_block) {}

Block::Block(void *handle) : OwnedObject(handle, &MaximFrontend::maxim_destroy_block) {}

bool Block::compile(uint64_t id, const QString &name, const QString &code, MaximCompiler::Block *blockOut,
                    MaximCompiler::Error *errorOut) {
    void *out = nullptr;
    auto compileSuccess =
        MaximFrontend::maxim_compile_block(id, name.toUtf8().constData(), code.toUtf8().constData(), &out, &out);

    if (compileSuccess) {
        *blockOut = Block(out);
    } else {
        *errorOut = Error(out);
    }

    return compileSuccess;
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
