#include "GroupNode.h"

using namespace MaximRuntime;

GroupNode::GroupNode(Schematic *parent) : Node(parent), _subsurface(parent->runtime(), parent, parent->depth() + 1) {

}

void GroupNode::compile() {
    instFunc()->reset();
    instFunc()->addInstantiable(_subsurface.instFunc());
    instFunc()->complete();

    Node::compile();
}
