#include "NodeControl.h"

#include "../node/Node.h"
#include "../connection/ConnectionWire.h"

using namespace AxiomModel;

NodeControl::NodeControl(Node *node, QString name, QPoint pos, QSize size)
        : GridItem(&node->surface, pos, size), node(node), m_name(std::move(name)) {

    connect(this, &NodeControl::selected,
            [this, node]() { node->select(true); });
}

void NodeControl::setName(const QString &name) {
    if (name != m_name) {
        m_name = name;
        emit nameChanged(name);
    }
}

void NodeControl::setShowName(bool showName) {
    if (showName != m_showName) {
        m_showName = showName;
        emit showNameChanged(showName);
    }
}

void NodeControl::initSink() {
    connect(this, &NodeControl::posChanged,
            this, &NodeControl::recalcSinkPos);
    connect(this, &NodeControl::sizeChanged,
            this, &NodeControl::recalcSinkPos);
    connect(node, &Node::posChanged,
            this, &NodeControl::recalcSinkPos);

    connect(this, &NodeControl::removed,
            sink(), &ConnectionSink::removed);
    connect(sink(), &ConnectionSink::connectionAdded,
            [this](ConnectionWire *wire) {
                connect(this, &NodeControl::removed,
                        wire, &ConnectionWire::remove);
            });

    recalcSinkPos();
}

void NodeControl::recalcSinkPos() {
    auto worldPos = pos() + NodeSurface::schematicToNodeSurface(node->pos());
    auto centerPos = worldPos + QPointF(size().width() / 2., size().height() / 2.);
    sink()->setPos(NodeSurface::nodeSurfaceToSchematicFloor(centerPos.toPoint()), centerPos);
}
