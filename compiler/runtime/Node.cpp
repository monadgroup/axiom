#include "Node.h"

using namespace MaximRuntime;

Node::Node(Schematic *parent) : CompileUnit(parent->runtime()), _parent(parent) {
    parent->addNode(this);
}

Node::~Node() {
    _parent->removeNode(this);
}
