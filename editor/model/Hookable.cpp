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

HookContext::HookContext(const AxiomModel::HookContext &a) {}

HookContext::HookContext(AxiomModel::HookContext &&a) noexcept {
    a.doDestruct();
    a.notifiables.clear();
}

HookContext& HookContext::operator=(const AxiomModel::HookContext &a) { return *this; }

HookContext& HookContext::operator=(AxiomModel::HookContext &&a) noexcept {
    a.doDestruct();
    a.notifiables.clear();
    return *this;
}

HookContext::~HookContext() {
    doDestruct();
}

void HookContext::addDestructHook(AxiomModel::HookNotifiable *handle) {
    notifiables.emplace(handle);
}

void HookContext::removeDestructHook(AxiomModel::HookNotifiable *handle) {
    notifiables.erase(handle);
}

void HookContext::doDestruct() {
    if (notifiables.empty()) return;
    std::vector<HookNotifiable*> notifyCopy(notifiables.begin(), notifiables.end());

    for (const auto &notifiable : notifyCopy) {
        notifiable->hookableDestroyed(this);
    }
}
