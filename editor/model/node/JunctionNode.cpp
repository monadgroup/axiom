#include "JunctionNode.h"

#include "../schematic/Schematic.h"

using namespace AxiomModel;

JunctionNode::JunctionNode(Schematic *parent, QPoint pos) : GridItem(parent, pos, QSize(1, 1)) {

}
