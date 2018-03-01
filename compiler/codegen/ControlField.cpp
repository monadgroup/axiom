#include "ControlField.h"

#include "Control.h"
#include "Type.h"

using namespace MaximCodegen;

ControlField::ControlField(Control *control, const std::string &name, Type *type)
    : ModuleClass(control->ctx(), control->module(), name), _type(type), _constructor(this, "constructor"),
      _getValue(this, "getValue", type->get()), _setValue(this, "setValue", nullptr, {type->get()}) {

}

void ControlField::doComplete() {
    _setValue.builder().CreateRetVoid();

    ModuleClass::doComplete();
}
