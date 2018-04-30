#include "GroupSchematic.h"

#include "../Project.h"
#include "../node/GroupNode.h"
#include "../control/NodeControl.h"

using namespace AxiomModel;

GroupSchematic::GroupSchematic(GroupNode *node) : Schematic(node->parentSchematic->project()), node(node) {
    connect(node, &GroupNode::nameChanged,
            this, &GroupSchematic::nameChanged);
    connect(node, &GroupNode::removed,
            this, [this]() {
                DO_SUPPRESS(project()->history, {
                    remove();
                });
            });
}

QString GroupSchematic::name() {
    return node->name();
}

SurfaceRef GroupSchematic::ref() const {
    auto nodeRef = node->ref();
    return SurfaceRef(nodeRef.surface.root, nodeRef.path());
}

void GroupSchematic::exposeControl(AxiomModel::NodeControl *control) {
    node->exposeControl(control);
    //project()->build();
}
