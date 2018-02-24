#include "NodeControl.h"

#include <cassert>

#include "NodeNumControl.h"
#include "../node/Node.h"
#include "../connection/ConnectionWire.h"
#include "compiler/runtime/Control.h"

using namespace AxiomModel;

NodeControl::NodeControl(Node *node, MaximRuntime::Control *runtime, QPoint pos, QSize size)
    : GridItem(&node->surface, pos, size), node(node), _runtime(runtime) {

    connect(this, &NodeControl::selected,
            [this, node]() { node->select(true); });
    connect(runtime, &MaximRuntime::Control::removed,
            this, &NodeControl::remove);
}

std::unique_ptr<NodeControl> NodeControl::fromRuntimeControl(Node *node, MaximRuntime::Control *runtime) {
    QSize newSize;

    switch (runtime->type()) {
        case MaximCommon::ControlType::NUMBER:
            newSize = QSize(2, 2);
            break;
        default:
            assert(false);
    }

    auto newPos = node->surface.grid.findNearestAvailable(QPoint(0, 0), newSize);

    switch (runtime->type()) {
        case MaximCommon::ControlType::NUMBER:
            return std::make_unique<NodeNumControl>(node, runtime, newPos, newSize);
        default:
            assert(false);
            throw;
    }
}

QString NodeControl::name() const {
    return QString::fromStdString(_runtime->name());
}

std::unique_ptr<GridItem> NodeControl::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    assert(false);
    throw;
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
