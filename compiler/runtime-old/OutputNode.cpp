#include "OutputNode.h"

#include "Runtime.h"

using namespace MaximRuntime;

OutputNode::OutputNode(Schematic *parent) : Node(parent), _control(this) {

}

OutputNode::~OutputNode() = default;

void OutputNode::compile() {
    inst()->reset();
    inst()->addInstantiable(&_control);
    inst()->complete();
}

void OutputNode::remove() {
    _control.remove();
    Node::remove();
}
