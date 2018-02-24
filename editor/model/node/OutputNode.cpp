#include "OutputNode.h"

#include "../schematic/Schematic.h"
#include "../control/NodeOutputControl.h"

#include "../schematic/RootSchematic.h"

using namespace AxiomModel;

OutputNode::OutputNode(RootSchematic *parent, MaximRuntime::OutputNode *runtime, QPoint pos)
    : Node(parent, "Output", Type::OUTPUT, pos, QSize(1, 1)), _runtime(runtime) {
    surface.addItem(std::make_unique<NodeOutputControl>(this, runtime->control()));
}

std::unique_ptr<GridItem> OutputNode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    assert(false);
    throw;
}
