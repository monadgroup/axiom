#include "ModuleSchematic.h"

#include "ModuleNode.h"

using namespace AxiomModel;

ModuleSchematic::ModuleSchematic(ModuleNode *node) : node(node) {
}

std::string ModuleSchematic::getName() {
    return node->name;
}
