#pragma once

#include "ModuleClass.h"

namespace MaximCodegen {

    class Control;

    class Type;

    class ControlField : public ModuleClass {
    public:
        ControlField(Control *control, const std::string &name, Type *type);

        ModuleClassMethod *constructor() override { return &_constructor; }

        ModuleClassMethod *getValue() { return &_getValue; }

        ModuleClassMethod *setValue() { return &_setValue; }

    private:

        ModuleClassMethod _constructor;
        ModuleClassMethod _getValue;
        ModuleClassMethod _setValue;
    };

}
