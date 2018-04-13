#include "IONode.h"

#include "../schematic/Schematic.h"
#include "editor/model/control/NodeIOControl.h"

#include "../schematic/RootSchematic.h"

using namespace AxiomModel;

IONode::IONode(RootSchematic *parent, const QString &name, QPoint pos, MaximCommon::ControlType type)
    : Node(parent, name, Type::IO, pos, QSize(1, 1)) {
    auto control = std::make_unique<NodeIOControl>(this, type, "");
    _control = control.get();
    surface.addItem(std::move(control));
}

std::unique_ptr<GridItem> IONode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    assert(false);
    throw;
}

void IONode::attachRuntime(MaximRuntime::IONode *runtime) {
    assert(!_runtime);
    _runtime = runtime;
    _control->attachRuntime(runtime->control());
}
