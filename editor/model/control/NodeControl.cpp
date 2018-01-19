#include "NodeControl.h"

#include "../node/Node.h"
#include "../connection/ConnectionWire.h"

using namespace AxiomModel;

NodeControl::NodeControl(Node *node, Channel channel, QPoint pos, QSize size)
        : GridItem(&node->surface, pos, size), node(node), channel(channel) {
    connect(this, &NodeControl::posChanged,
            this, &NodeControl::recalcWorldPos);
    connect(node, &Node::posChanged,
            this, &NodeControl::recalcWorldPos);
    connect(this, &NodeControl::selected,
            [this, node]() { node->select(true); });
    connect(this, &NodeControl::worldPosChanged,
            &sink, &ConnectionSink::setPos);

    connect(&sink, &ConnectionSink::connectionAdded,
            [this](ConnectionWire *wire) {
                connect(this, &NodeControl::removed,
                        wire, &ConnectionWire::remove);
            });
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
