#include "Hookable.h"

using namespace AxiomModel;

Hookable::Hookable(AxiomModel::Hookable &&a) noexcept {
    destructEvents = std::move(a.destructEvents);
}

Hookable& Hookable::operator=(AxiomModel::Hookable &&a) noexcept {
    triggerDestruct(true);
    destructEvents = std::move(a.destructEvents);
    return *this;
}

Hookable::~Hookable() {
    triggerDestruct(true);
}

void Hookable::addDestructHook(void *handle, std::function<void()> func) {
    destructEvents.emplace(handle, std::move(func));
}

void Hookable::removeDestructHook(void *handle) {
    destructEvents.erase(handle);
}

void Hookable::triggerDestruct(bool isFinal) {
    for (const auto &event : destructEvents) {
        event.second();
    }

    if (!isFinal) destructEvents.clear();
}
