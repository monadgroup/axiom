#include "Node.h"

#include "Surface.h"

using namespace MaximRuntime;

Node::Node(Surface *surface) : ModuleRuntimeUnit(surface->runtime(), "node"), _surface(surface) {

}

void Node::remove() {
    _surface->removeNode(this);
}

void Node::scheduleCompile() {
    _needsCompile = true;
    _surface->scheduleGraphUpdate();
}
