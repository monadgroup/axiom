#include "ModuleSchematic.h"

#include "editor/model/node/ModuleNode.h"

using namespace AxiomModel;

ModuleSchematic::ModuleSchematic(ModuleNode *node) : Schematic(node->runtime()->subsurface()), node(node) {
    connect(node, &ModuleNode::nameChanged,
            this, &ModuleSchematic::nameChanged);
}

QString ModuleSchematic::name() {
    return node->name();
}
