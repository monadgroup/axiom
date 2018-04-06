#include "GroupSchematic.h"

#include "../Project.h"
#include "../node/GroupNode.h"
#include "../control/NodeControl.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

GroupSchematic::GroupSchematic(GroupNode *node) : Schematic(node->parentSchematic->project(), SurfaceRef(node->ref().path()), node->runtime()->subsurface()), node(node) {
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
    project()->build();
}
