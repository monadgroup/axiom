#include "ModuleNode.h"

#include "compiler/runtime/GeneratableModuleClass.h"
#include "compiler/runtime/Runtime.h"
#include "../control/NodeControl.h"

using namespace AxiomModel;

ModuleNode::ModuleNode(Schematic *parent, QString name, QPoint pos, QSize size)
    : Node(parent, std::move(name), Type::GROUP, pos, size),
      schematic(std::make_unique<ModuleSchematic>(this)), _runtime(parent->runtime()) {
    connect(this, &ModuleNode::removed,
            schematic.get(), &ModuleSchematic::removed);

    connect(this, &ModuleNode::removed,
            [this]() {
                _runtime.remove();
                _runtime.runtime()->compile();
            });

    connect(&_runtime, &MaximRuntime::GroupNode::controlAdded,
            this, &ModuleNode::controlAdded);
}

std::unique_ptr<GridItem> ModuleNode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    auto schematicParent = dynamic_cast<Schematic *>(newParent);
    assert(schematicParent != nullptr);

    auto moduleNode = std::make_unique<ModuleNode>(schematicParent, name(), pos(), size());
    surface.cloneTo(&moduleNode->surface);
    schematic->cloneTo(moduleNode->schematic.get());
    return std::move(moduleNode);
}

void ModuleNode::controlAdded(MaximRuntime::SoftControl *control) {
    auto newControl = NodeControl::fromRuntimeControl(this, control);
    // todo: set newControl's exposeBase to the Model Control this SoftControl is for
    // (this info is currently only needed on serialize, so there might be a better way to do it -- maybe the model
    // should be in charge of creating forward controls instead of the compiler?)
    // --> the same might go for other controls, when we need to deserialize
    surface.addItem(std::move(newControl));
}
