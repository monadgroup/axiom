#include "ModuleSchematic.h"

#include "ModuleNode.h"

using namespace AxiomModel;

ModuleSchematic::ModuleSchematic(ModuleNode *node) : node(node) {
}

QString ModuleSchematic::name() {
    return node->name();
}
