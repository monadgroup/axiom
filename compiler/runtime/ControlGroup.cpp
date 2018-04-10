#include <iostream>
#include "ControlGroup.h"

#include "Surface.h"
#include "Control.h"
#include "Runtime.h"
#include "../codegen/Control.h"

using namespace MaximRuntime;

ControlGroup::ControlGroup(Surface *surface, MaximCodegen::Control *type)
    : RuntimeUnit(surface->runtime()), _type(type), _surface(surface),
      _compileResult(surface->runtime()->ctx(), surface->module(), "controlgroup", nullptr) {

}

llvm::Module *ControlGroup::module() {
    return surface()->module();
}

MaximCodegen::ModuleClass *ControlGroup::compile() {
    auto storeType = type()->underlyingType();
    if (extracted()) {
        _compileResult.setStorageType(llvm::ArrayType::get(storeType, MaximCodegen::ArrayType::arraySize));
    } else {
        _compileResult.setStorageType(storeType);
    }

    return &_compileResult;
}

void ControlGroup::absorb(ControlGroup *other) {
    if (other == this) return;

    auto otherControls = std::set<Control *>(other->controls());
    for (const auto &control : otherControls) {
        control->setGroup(this);
    }
}

void ControlGroup::addControl(Control *control) {
    _controls.emplace(control);
}

void ControlGroup::removeControl(Control *control) {
    _controls.erase(control);
}

bool ControlGroup::exposed() const {
    for (const auto &control : _controls) {
        if (control->exposer()) return true;
    }
    return false;
}

bool ControlGroup::writtenTo() const {
    for (const auto &control : _controls) {
        if (control->writtenTo()) return true;
    }
    return false;
}

bool ControlGroup::readFrom() const {
    for (const auto &control : _controls) {
        if (control->readFrom()) return true;
    }
    return false;
}

NumValue ControlGroup::getNumValue() const {
    return _surface->runtime()->op().readNum(currentValuePtr());
}

void ControlGroup::setNumValue(const NumValue &value) const {
    _surface->runtime()->op().writeNum(currentValuePtr(), value);
}

MidiValue ControlGroup::getMidiValue() const {
    return _surface->runtime()->op().readMidi(currentValuePtr());
}

void ControlGroup::setMidiValue(const MidiValue &value) const {
    _surface->runtime()->op().writeMidi(currentValuePtr(), value);
}

void ControlGroup::pushMidiEvent(const MidiEventValue &event) const {
    _surface->runtime()->op().pushMidiEvent(currentValuePtr(), event);
}

uint32_t ControlGroup::getActiveFlags() const {
    if (_type->type() == MaximCommon::ControlType::NUMBER || _type->type() == MaximCommon::ControlType::NUM_EXTRACT) {
        return _surface->runtime()->op().readNumArrayActiveFlags(currentValuePtr());
    } else if (_type->type() == MaximCommon::ControlType::MIDI ||
               _type->type() == MaximCommon::ControlType::MIDI_EXTRACT) {
        return _surface->runtime()->op().readMidiArrayActiveFlags(currentValuePtr());
    } else {
        assert(false);
        return 0;
    }
}
