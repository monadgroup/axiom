#include "ModuleNode.h"
#include "compiler/runtime/SoftControl.h"

using namespace AxiomModel;

ModuleNode::ModuleNode(Schematic *parent, QString name, QPoint pos, QSize size)
        : Node(parent, std::move(name), Type::GROUP, pos, size),
          schematic(std::make_unique<ModuleSchematic>(this)), _node(parent->runtime()) {
    connect(this, &ModuleNode::removed,
            schematic.get(), &ModuleSchematic::removed);
}

std::unique_ptr<GridItem> ModuleNode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    auto schematicParent = dynamic_cast<Schematic *>(newParent);
    assert(schematicParent != nullptr);

    auto moduleNode = std::make_unique<ModuleNode>(schematicParent, name(), pos(), size());
    surface.cloneTo(&moduleNode->surface);
    schematic->cloneTo(moduleNode->schematic.get());
    return std::move(moduleNode);
}
