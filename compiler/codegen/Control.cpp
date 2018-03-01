#include "Control.h"

#include "Type.h"

using namespace MaximCodegen;

Control::Control(MaximContext *ctx, llvm::Module *module, llvm::Type *storageType, const std::string &name)
    : ModuleClass(ctx, module, name), _storageType(storageType), _constructor(this, "constructor") {

}

ControlField* Control::addField(const std::string &name, Type *type) {
    auto index = _fields.emplace(name, ControlField(this, name, type));
    auto field = &index.first->second;
    field->constructor()->call(constructor()->builder(), {}, constructor()->contextPtr(), module(), "");
    return field;
}

llvm::Constant* Control::initializeVal() {
    return llvm::UndefValue::get(storageType());
}

llvm::Type* Control::storageType() {
    return llvm::PointerType::get(_storageType, 0);
}

void Control::doComplete() {
    for (const auto &pair : _fields) {
        pair.second.complete();
    }

    ModuleClass::doComplete();
}
