#include "IONode.h"

#include "../schematic/Schematic.h"
#include "editor/model/control/NodeIOControl.h"

#include "../schematic/RootSchematic.h"

using namespace AxiomModel;

IONode::IONode(RootSchematic *parent, size_t index, MaximRuntime::IONode *runtime, const QString &name, QPoint pos)
    : Node(parent, index, name, Type::IO, pos, QSize(1, 1)), _runtime(runtime) {
    surface.addItem(std::make_unique<NodeIOControl>(this, 0, runtime->control()));
}

std::unique_ptr<GridItem> IONode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    assert(false);
    throw;
}
