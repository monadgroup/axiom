#pragma once

#include "ModuleClass.h"
#include "ControlClassMethod.h"

namespace MaximCodegen {

    class Control;

    class Type;

    class ControlField : public ModuleClass {
    public:
        ControlField(Control *control, const std::string &name, Type *type);

        ControlField(ControlField &&oldField) = delete;

        Type *type() const { return _type; }

        ControlClassMethod *constructor() override { return &_constructor; }

        ControlClassMethod *getValue() { return &_getValue; }

        ControlClassMethod *setValue() { return &_setValue; }

        llvm::Type *storageType() override;

    protected:

        void doComplete() override;

    private:
        Control *_control;
        Type *_type;

        ControlClassMethod _constructor;
        ControlClassMethod _getValue;
        ControlClassMethod _setValue;
    };

}
