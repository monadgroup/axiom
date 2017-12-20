#include "ModuleSchematic.h"

#include "ModuleNode.h"

using namespace AxiomModel;

ModuleSchematic::ModuleSchematic(ModuleNode *node) : node(node) {
}

QString ModuleSchematic::getName() {
    return node->name;
}
