#pragma once

#include <set>
#include <memory>

#include "codegen/Instantiable.h"
#include "../common/ControlType.h"
#include "CompileLeaf.h"
#include "Schematic.h"

namespace MaximRuntime {

    class Control;


    class ControlGroup : public MaximCodegen::Instantiable, public CompileLeaf {
    public:

        ControlGroup(MaximCommon::ControlType type, Schematic *initialSchematic);

        MaximCommon::ControlType type() const { return _type; }

        Schematic *parentUnit() const override { return _schematic; }

        std::set<Control*> &controls() { return _controls; }

        void absorb(ControlGroup *other);

        void addControl(Control *control);

        void removeControl(Control *control);

        MaximCodegen::Instantiable *inst() override { return this; }

        llvm::Constant *getInitialVal(MaximCodegen::MaximContext *ctx) override;

        void initializeVal(MaximCodegen::MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, MaximCodegen::InstantiableFunction *parent, MaximCodegen::Builder &b) override;

        llvm::Type *type(MaximCodegen::MaximContext *ctx) const override;

    private:

        MaximCommon::ControlType _type;

        Schematic *_schematic;

        std::set<Control*> _controls;

        llvm::GlobalVariable *_global = nullptr;

        void setSchematic(Schematic *newSchematic);

    };

}
