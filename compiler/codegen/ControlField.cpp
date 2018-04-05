#include "ControlField.h"

#include "Control.h"
#include "Type.h"

using namespace MaximCodegen;

ControlField::ControlField(Control *control, const std::string &name, Type *type)
    : ModuleClass(control->ctx(), control->module(), control->name() + "." + name), _control(control), _type(type),
      _constructor(this, "constructor"), _getValue(this, "getValue", llvm::PointerType::get(type->get(), 0)),
      _setValue(this, "setValue", nullptr, {llvm::PointerType::get(type->get(), 0)}) {

}

llvm::Type *ControlField::storageType() {
    return _control->storageType();
}

void ControlField::doComplete() {
    _setValue.builder().CreateRetVoid();

    ModuleClass::doComplete();
}
