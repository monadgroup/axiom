#include "OutputNode.h"

#include "Surface.h"
#include "Runtime.h"
#include "GeneratableModuleClass.h"

using namespace MaximRuntime;

OutputNode::OutputNode(Surface *surface) : Node(surface) {
    _moduleClass = std::make_unique<GeneratableModuleClass>(surface->runtime()->ctx(), module(), "outputnode");
}

GeneratableModuleClass* OutputNode::compile() {
    return _moduleClass.get();
}

void OutputNode::remove() {
    _control->remove();
    Node::remove();
}
