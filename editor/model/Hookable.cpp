#include "Hookable.h"

#include <vector>

using namespace AxiomModel;

/**
 * Copy/move strategy:
 * In general we don't want to copy across notifiables, as they're only expecting to be notified
 * on destruction _from the original object_. Since after a move the original object will be in
 * an unspecified state, we treat it as if it's being destructed at that point and immediately
 * fire destruct notifiables. To make sure these notifiables aren't called again, the set must
 * be cleared.
 */

Hookable::Hookable(const AxiomModel::Hookable &a) {}

Hookable::Hookable(AxiomModel::Hookable &&a) noexcept {
    a.doDestruct();
    a.notifiables.clear();
}

Hookable& Hookable::operator=(const AxiomModel::Hookable &a) { return *this; }

Hookable& Hookable::operator=(AxiomModel::Hookable &&a) noexcept {
    a.doDestruct();
    a.notifiables.clear();
    return *this;
}

Hookable::~Hookable() {
    doDestruct();
}

void Hookable::addDestructHook(AxiomModel::HookNotifiable *handle) {
    notifiables.emplace(handle);
}

void Hookable::removeDestructHook(AxiomModel::HookNotifiable *handle) {
    notifiables.erase(handle);
}

void Hookable::doDestruct() {
    if (notifiables.empty()) return;
    std::vector<HookNotifiable*> notifyCopy(notifiables.begin(), notifiables.end());

    for (const auto &notifiable : notifyCopy) {
        notifiable->hookableDestroyed(this);
    }
}
