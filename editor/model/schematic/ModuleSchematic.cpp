#include "ModuleSchematic.h"

#include "editor/model/node/ModuleNode.h"
#include "editor/model/control/NodeControl.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

ModuleSchematic::ModuleSchematic(ModuleNode *node) : Schematic(node->runtime()->subsurface()), node(node) {
    connect(node, &ModuleNode::nameChanged,
            this, &ModuleSchematic::nameChanged);
}

QString ModuleSchematic::name() {
    return node->name();
}

void ModuleSchematic::exposeControl(AxiomModel::NodeControl *control) {
    node->runtime()->forwardControl(control->runtime());
    node->runtime()->runtime()->compile();
}
