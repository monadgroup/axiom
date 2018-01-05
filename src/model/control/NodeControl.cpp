#include "NodeControl.h"

#include "../node/Node.h"

using namespace AxiomModel;

NodeControl::NodeControl(Node *node, Channel channel) : GridItem(&parent->surface), node(node), channel(channel) {
    connect(this, &NodeControl::posChanged,
            this, &NodeControl::recalcWorldPos);
    connect(node, &Node::posChanged,
            this, &NodeControl::recalcWorldPos);
}

void NodeControl::setName(const QString &name) {
    if (name != m_name) {
        m_name = name;
        emit nameChanged(name);
    }
}

void NodeControl::recalcWorldPos() {
    auto newWorldPos = pos() + node->pos();
    if (newWorldPos != m_worldPos) {
        m_worldPos = newWorldPos;
        emit worldPosChanged(newWorldPos);
    }
}
