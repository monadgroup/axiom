#include "OutputNode.h"

#include <cassert>

#include "../schematic/Schematic.h"
#include "../control/NodeOutputControl.h"
#include "compiler/runtime/OutputNode.h"

using namespace AxiomModel;

OutputNode::OutputNode(Schematic *parent, QPoint pos)
    : Node(parent, "Output", Type::OUTPUT, pos, QSize(1, 1)), _runtime(parent->runtime()) {
    surface.addItem(std::make_unique<NodeOutputControl>(this, _runtime.control()));
}

std::unique_ptr<GridItem> OutputNode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    assert(false);
    throw;
}
