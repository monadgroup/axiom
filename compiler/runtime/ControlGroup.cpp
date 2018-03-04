#include "ControlGroup.h"

#include "Surface.h"
#include "Control.h"

using namespace MaximRuntime;

ControlGroup::ControlGroup(Surface *surface, MaximCodegen::Control *type)
    : RuntimeUnit(surface->runtime(), surface->module()), _type(type), _surface(surface) {

}

void ControlGroup::absorb(ControlGroup *other) {
    auto otherControls = std::set<Control*>(other->controls());
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
