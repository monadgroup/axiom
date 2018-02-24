#include "ControlGroup.h"

#include "Node.h"
#include "Control.h"
#include "Runtime.h"
#include "codegen/Control.h"

using namespace MaximRuntime;

ControlGroup::ControlGroup(MaximCommon::ControlType type, Schematic *initialSchematic)
    : CompileLeaf(initialSchematic->runtime()), _type(type), _schematic(initialSchematic) {

}

void ControlGroup::absorb(ControlGroup *other) {
    auto otherControls = std::set<Control *>(other->controls());
    for (const auto &control : otherControls) {
        control->setGroup(this);
    }
}

void ControlGroup::addControl(Control *control) {
    _controls.emplace(control);

    auto controlSchematic = control->node()->parentUnit();
    if (controlSchematic->depth() < _schematic->depth()) {
        setSchematic(controlSchematic);
    }
}

void ControlGroup::removeControl(Control *control) {
    // remove control from list
    _controls.erase(control);

    // if we're empty, remove self from parent
    // note: our destructor is called from this!
    if (_controls.empty()) {
        _schematic->removeControlGroup(this);
        return;
    }

    // otherwise, find the lowest-depth schematic in our controls (it might've changed)
    Schematic *newSchematic = nullptr;
    for (const auto &indexedControl : _controls) {
        auto controlSchematic = indexedControl->node()->parentUnit();
        if (!newSchematic || controlSchematic->depth() < newSchematic->depth()) {
            newSchematic = controlSchematic;
        }
    }
    assert(newSchematic);

    if (_schematic != newSchematic) {
        setSchematic(newSchematic);
    }
}

llvm::Constant *ControlGroup::getInitialVal(MaximCodegen::MaximContext *ctx) {
    switch (type()) {
        case MaximCommon::ControlType::NUMBER:
            return llvm::ConstantStruct::get(ctx->numType()->get(), {
                llvm::ConstantVector::get({ctx->constFloat(0), ctx->constFloat(0)}),
                llvm::ConstantInt::get(ctx->numType()->formType(), (uint64_t) MaximCommon::FormType::CONTROL, false),
                llvm::ConstantInt::get(ctx->numType()->activeType(), (uint64_t) true, false)
            });
        default:
            assert(false);
            throw;
    }
}

void ControlGroup::initializeVal(MaximCodegen::MaximContext *ctx, llvm::Module *module, llvm::Value *ptr,
                                 MaximCodegen::InstantiableFunction *parent, MaximCodegen::Builder &b) {
    for (const auto &control : _controls) {
        auto codegenControl = control->control();
        if (!codegenControl) continue;

        // todo: need a better way to do this: it's slow (requires searching the entire subtree each time)
        // and won't work with nodes/controls that are instanced multiple times
        auto controlPtr = parent->getInitializePointer(codegenControl);
        assert(controlPtr->getType()->isPointerTy()
               && controlPtr->getType()->getPointerElementType()->isPointerTy()
               && controlPtr->getType()->getPointerElementType()->getPointerElementType() == type(ctx));

        b.CreateStore(ptr, controlPtr);
    }
}

llvm::Type *ControlGroup::type(MaximCodegen::MaximContext *ctx) const {
    switch (type()) {
        case MaximCommon::ControlType::NUMBER:
            return ctx->numType()->get();
        default:
            assert(false);
            throw;
    }
}

NumValue ControlGroup::getNumValue() const {
    return _schematic->runtime()->op()->readNum(currentPtr());
}

void ControlGroup::setNumValue(NumValue value) const {
    _schematic->runtime()->op()->writeNum(currentPtr(), value);
}

void ControlGroup::setSchematic(Schematic *newSchematic) {
    auto owner = _schematic->removeControlGroup(this);
    newSchematic->addControlGroup(std::move(owner));
    _schematic = newSchematic;

    _global->removeFromParent();
    getValueFunction->removeFromParent();
    getValueFunction = nullptr;
}
