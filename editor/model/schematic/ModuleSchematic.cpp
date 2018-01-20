#include "ModuleSchematic.h"

#include "editor/model/node/ModuleNode.h"

using namespace AxiomModel;

ModuleSchematic::ModuleSchematic(ModuleNode *node) : node(node) {
    connect(node, &ModuleNode::nameChanged,
            this, &ModuleSchematic::nameChanged);
}

QString ModuleSchematic::name() {
    return node->name();
}
