#include "Control.h"

#include "Type.h"
#include "MaximContext.h"

using namespace MaximCodegen;

Control::Control(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, llvm::Type *storageType,
                 llvm::Type *valueType, llvm::Type *underlyingType, const std::string &name)
    : ModuleClass(ctx, module, name), _type(type), _storageType(storageType), _valueType(valueType),
      _underlyingType(underlyingType), _constructor(this, "constructor"), _update(this, "update") {

}

ControlField *Control::addField(const std::string &name, Type *type) {
    auto index = _fields.emplace(name, std::make_unique<ControlField>(this, name, type));
    auto field = index.first->second.get();
    field->constructor()->call(constructor()->builder(), {}, constructor()->contextPtr(), module(), "");
    return field;
}

ControlField *Control::getField(const std::string &name) {
    auto index = _fields.find(name);
    if (index != _fields.end()) return index->second.get();
    else return nullptr;
}

llvm::Type *Control::storageType() {
    return llvm::StructType::get(ctx()->llvm(), {
        llvm::PointerType::get(_valueType, 0),
        _storageType
    });
}

void Control::doComplete() {
    for (auto &pair : _fields) {
        pair.second->complete();
    }
    _update.builder().CreateRetVoid();

    ModuleClass::doComplete();
}
