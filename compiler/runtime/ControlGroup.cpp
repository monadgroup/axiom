#include "ControlGroup.h"

#include <llvm/IR/GlobalVariable.h>

#include "../codegen/MaximContext.h"
#include "Control.h"
#include "Node.h"
#include "Surface.h"

using namespace MaximRuntime;

size_t ControlGroup::_nextId = 0;

ControlGroup::ControlGroup(Surface *surface, MaximCommon::ControlType type, Control *writer, std::vector<Control *> controls, bool isExposed)
    : _surface(surface), _type(type), _writer(writer), _controls(std::move(controls)), _isExposed(isExposed), _id(_nextId++) {
    surface->groups().push_back(this);
    setupGlobal();
}

ControlGroup::~ControlGroup() {
    auto index = std::find(_surface->groups().begin(), _surface->groups().end(), this);
    if (index != _surface->groups().end()) _surface->groups().erase(index);

    _global->removeFromParent();
}

ControlGroup::ControlGroup(Control *control) : _surface(control->node()->surface()), _controls({control}), _id(_nextId++) {
    if (control->direction() == MaximCommon::ControlDirection::OUT) _writer = control;
    if (control->exposer()) _isExposed = true;
    setupGlobal();
}

void ControlGroup::findWriter(Control *ignore) {
    for (const auto &control : _controls) {
        if (control == ignore || control->direction() != MaximCommon::ControlDirection::OUT) continue;
        _writer = control;
        _surface->markAsDirty();
        break;
    }
}

void ControlGroup::updateExposed() {
    _isExposed = false;
    for (const auto &control : _controls) {
        if (!control->exposer()) continue;
        _isExposed = true;
        _surface->markAsDirty();
        break;
    }
}

void ControlGroup::setupGlobal() {
    auto globalName = "control." + std::to_string(_id);

    // todo: refactor somewhere else
    llvm::Constant *_globalIni;
    switch (_type) {
        case MaximCommon::ControlType::NUMBER:
            _globalIni = llvm::ConstantStruct::get(_surface->context()->numType()->get(), {
                llvm::ConstantVector::getSplat(2, _surface->context()->constFloat(0)),
                llvm::ConstantInt::get(_surface->context()->numType()->formType(), (uint64_t) MaximCommon::FormType::LINEAR, false),
                llvm::ConstantInt::get(_surface->context()->numType()->activeType(), (uint64_t) false, false)
            });
            break;
        default:
            assert(false);
            throw;
    }

    _global = new llvm::GlobalVariable(
        *_surface->module(),
        _globalIni->getType(),
        false,
        llvm::GlobalVariable::LinkageTypes::ExternalLinkage,
        _globalIni,
        globalName
    );
}
