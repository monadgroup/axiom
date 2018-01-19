#include "NodeControl.h"

#include "../node/Node.h"
#include "../connection/ConnectionWire.h"
#include "editor/util.h"

using namespace AxiomModel;

NodeControl::NodeControl(Node *node, Channel channel, QPoint pos, QSize size)
        : GridItem(&node->surface, pos, size), node(node), channel(channel) {
    connect(this, &NodeControl::posChanged,
            this, &NodeControl::recalcSinkPos);
    connect(node, &Node::posChanged,
            this, &NodeControl::recalcSinkPos);
    connect(this, &NodeControl::selected,
            [this, node]() { node->select(true); });
    connect(this, &NodeControl::removed,
            &sink, &ConnectionSink::removed);

    connect(&sink, &ConnectionSink::connectionAdded,
            [this](ConnectionWire *wire) {
                connect(this, &NodeControl::removed,
                        wire, &ConnectionWire::remove);
            });

    recalcSinkPos();
}

void NodeControl::setName(const QString &name) {
    if (name != m_name) {
        m_name = name;
        emit nameChanged(name);
    }
}

void NodeControl::recalcSinkPos() {
    auto worldPos = pos() + NodeSurface::schematicToNodeSurface(node->pos());
    auto centerPos = worldPos + QPointF(size().width() / 2., size().height() / 2.);
    sink.setPos(NodeSurface::nodeSurfaceToSchematicFloor(AxiomUtil::floorP(centerPos)), centerPos);
}
