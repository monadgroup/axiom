#include "Node.h"

#include "Surface.h"

using namespace MaximRuntime;

Node::Node(Surface *surface) : ModuleRuntimeUnit(surface->runtime(), "node"), _surface(surface) {
    scheduleCompile();
}

void Node::remove() {
    cleanup();
    _surface->removeNode(this);
}

void Node::scheduleCompile() {
    _needsCompile = true;
    _surface->scheduleGraphUpdate();
}

void Node::setExtracted(bool extracted) {
    if (extracted != _extracted) {
        _extracted = extracted;
        emit extractedChanged(extracted);
    }
}
