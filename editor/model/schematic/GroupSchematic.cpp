#include "GroupSchematic.h"

#include "editor/model/node/GroupNode.h"
#include "editor/model/control/NodeControl.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

GroupSchematic::GroupSchematic(GroupNode *node) : Schematic(node->runtime()->subsurface()), node(node) {
    connect(node, &GroupNode::nameChanged,
            this, &GroupSchematic::nameChanged);
    connect(node, &GroupNode::removed,
            this, &GroupSchematic::remove);
}

QString GroupSchematic::name() {
    return node->name();
}

void GroupSchematic::exposeControl(AxiomModel::NodeControl *control) {
    node->runtime()->forwardControl(control->runtime());
    node->runtime()->runtime()->compile();
}
