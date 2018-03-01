#pragma once

#include "ControlFieldClassMethod.h"

namespace MaximCodegen {

    class Control;

    class Type;

    class ControlField : public ModuleClass {
    public:
        ControlField(Control *control, const std::string &name, Type *type);

        ControlFieldClassMethod *constructor() override { return &_constructor; }

        ControlFieldClassMethod *getValue() { return &_getValue; }

        ControlFieldClassMethod *setValue() { return &_setValue; }

    protected:

        void doComplete() override;

    private:

        ControlFieldClassMethod _constructor;
        ControlFieldClassMethod _getValue;
        ControlFieldClassMethod _setValue;
    };

}
