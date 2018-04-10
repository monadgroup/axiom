#include "RootSurface.h"

using namespace MaximRuntime;

RootSurface::RootSurface(Runtime *runtime) : Surface(runtime, 0),
                                             input(this, MaximCommon::ControlType::MIDI, false, true),
                                             output(this, MaximCommon::ControlType::NUMBER, true, false) {

}

void *RootSurface::getValuePtr(void *parentCtx) {
    return parentCtx;
}

void RootSurface::addExitNodes(std::set<MaximRuntime::Node *> &queue) {
    queue.emplace(&output);
}
