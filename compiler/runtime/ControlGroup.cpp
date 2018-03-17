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
    auto otherControls = std::set<Control *>(other->controls());
    for (const auto &control : otherControls) {
        control->setGroup(this);
    }
}

void ControlGroup::addControl(Control *control) {
    _controls.emplace(control);
}

void ControlGroup::removeControl(Control *control) {
    // remove control from list
    _controls.erase(control);

    // if we're empty, remove self from parent
    // note: our destructor is called from this!
    if (_controls.empty()) {
        _surface->removeControlGroup(this);
        return;
    }
}

bool ControlGroup::exposed() const {
    for (const auto &control : _controls) {
        if (control->exposed()) return true;
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
    return _surface->runtime()->op().readNum(currentPtr());
}

void ControlGroup::setNumValue(const NumValue &value) const {
    _surface->runtime()->op().writeNum(currentPtr(), value);
}

MidiValue ControlGroup::getMidiValue() const {
    return _surface->runtime()->op().readMidi(currentPtr());
}

void ControlGroup::setMidiValue(const MidiValue &value) const {
    _surface->runtime()->op().writeMidi(currentPtr(), value);
}

void ControlGroup::pushMidiEvent(const MidiEventValue &event) const {
    _surface->runtime()->op().pushMidiEvent(currentPtr(), event);
}

uint32_t ControlGroup::getActiveFlags() const {
    if (_type->type() == MaximCommon::ControlType::NUMBER || _type->type() == MaximCommon::ControlType::NUM_EXTRACT) {
        return _surface->runtime()->op().readNumArrayActiveFlags(currentPtr());
    } else if (_type->type() == MaximCommon::ControlType::MIDI || _type->type() == MaximCommon::ControlType::MIDI_EXTRACT) {
        return _surface->runtime()->op().readMidiArrayActiveFlags(currentPtr());
    } else {
        assert(false);
        return 0;
    }
}
