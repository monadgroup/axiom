#include "OutputNode.h"

#include "Runtime.h"

using namespace MaximRuntime;

OutputNode::OutputNode(Schematic *parent) : Node(parent), _control(this) {

}

OutputNode::~OutputNode() = default;

void OutputNode::compile() {
    inst()->reset();

    auto ptr = inst()->addInstantiable(&_control);
    auto outputNum = inst()->builder().CreateLoad(ptr);
    inst()->builder().CreateStore(outputNum, runtime()->outputPtr(module()));

    inst()->complete();
}

void OutputNode::remove() {
    _control.remove();
    Node::remove();
}
