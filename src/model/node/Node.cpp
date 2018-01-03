#include "Node.h"

#include "src/model/schematic/Schematic.h"

using namespace AxiomModel;

Node::Node(Schematic *parent) : GridItem(parent) {

}

void Node::setName(const QString &name) {
    if (name != m_name) {
        m_name = name;
        emit nameChanged(name);
    }
}
