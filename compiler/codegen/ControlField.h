#pragma once

#include "ModuleClass.h"
#include "ControlFieldClassMethod.h"

namespace MaximCodegen {

    class Control;

    class Type;

    class ControlField : public ModuleClass {
    public:
        ControlField(Control *control, const std::string &name, Type *type);

        Type *type() const { return _type; }

        ControlFieldClassMethod *constructor() override { return &_constructor; }

        ControlFieldClassMethod *getValue() { return &_getValue; }

        ControlFieldClassMethod *setValue() { return &_setValue; }

    protected:

        void doComplete() override;

    private:
        Type *_type;

        ControlFieldClassMethod _constructor;
        ControlFieldClassMethod _getValue;
        ControlFieldClassMethod _setValue;
    };

}
