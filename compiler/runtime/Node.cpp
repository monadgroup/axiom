#include "Node.h"

using namespace MaximRuntime;

Node::Node(Schematic *parent) : CompileUnit(parent->runtime()), _parent(parent) {
    parent->addNode(this);
}

void Node::remove() {
    _parent->removeNode(this);
}
