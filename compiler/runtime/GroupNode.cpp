#include "GroupNode.h"
#include "GeneratableModuleClass.h"

using namespace MaximRuntime;

GroupNode::GroupNode(Surface *surface) : Node(surface), _subsurface(surface->runtime(), surface->depth() + 1) {

}
