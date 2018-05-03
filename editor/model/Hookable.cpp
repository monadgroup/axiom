#include "Hookable.h"

using namespace AxiomModel;

Hookable::Hookable(AxiomModel::Hookable &&a) noexcept {
    destructEvents = std::move(a.destructEvents);
}

Hookable& Hookable::operator=(AxiomModel::Hookable &&a) noexcept {
    triggerDestruct();
    destructEvents = std::move(a.destructEvents);
    return *this;
}

Hookable::~Hookable() {
    triggerDestruct();
}

void Hookable::addDestructHook(void *handle, std::function<void()> func) {
    destructEvents.emplace(handle, std::move(func));
}

void Hookable::removeDestructHook(void *handle) {
    destructEvents.erase(handle);
}

void Hookable::triggerDestruct() {
    for (const auto &event : destructEvents) {
        event.second();
    }
}
