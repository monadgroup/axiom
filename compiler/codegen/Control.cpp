#include "Control.h"

#include "Type.h"

using namespace MaximCodegen;

Control::Control(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, llvm::Type *storageType,
                 const std::string &name)
    : UndefInitializedModuleClass(ctx, module, name), _type(type), _storageType(storageType),
      _constructor(this, "constructor") {

}

ControlField* Control::addField(const std::string &name, Type *type) {
    auto index = _fields.emplace(name, std::make_unique<ControlField>(this, name, type));
    auto field = index.first->second.get();
    field->constructor()->call(constructor()->builder(), {}, constructor()->contextPtr(), module(), "");
    return field;
}

ControlField* Control::getField(const std::string &name) {
    auto index = _fields.find(name);
    if (index != _fields.end()) return index->second.get();
    else return nullptr;
}

llvm::Type* Control::storageType() {
    return llvm::PointerType::get(_storageType, 0);
}

void Control::doComplete() {
    for (auto &pair : _fields) {
        pair.second->complete();
    }

    ModuleClass::doComplete();
}
