#include "RootSurface.h"

using namespace MaximRuntime;

RootSurface::RootSurface(Runtime *runtime) : Surface(runtime, 0) {
    auto inputNode = std::make_unique<IONode>(this, MaximCommon::ControlType::MIDI, false, true);
    input = inputNode.get();
    addNode(std::move(inputNode));

    auto outputNode = std::make_unique<IONode>(this, MaximCommon::ControlType::NUMBER, true, false);
    output = outputNode.get();
    addNode(std::move(outputNode));
}

void *RootSurface::getValuePtr(void *parentCtx) {
    return parentCtx;
}

void RootSurface::addExitNodes(std::set<MaximRuntime::Node *> &queue) {
    queue.emplace(output);
}
