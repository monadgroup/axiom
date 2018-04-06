#include "NodeControl.h"

#include <cassert>

#include "NodeNumControl.h"
#include "NodeMidiControl.h"
#include "NodeExtractControl.h"
#include "../schematic/Schematic.h"
#include "../node/Node.h"
#include "../connection/ConnectionWire.h"
#include "compiler/runtime/Control.h"
#include "compiler/codegen/Control.h"

using namespace AxiomModel;

NodeControl::NodeControl(Node *node, size_t index, MaximRuntime::Control *runtime, QPoint pos, QSize size)
    : GridItem(&node->surface, pos, size), node(node), _ref(ControlRef(node->ref(), index)), _runtime(runtime) {

    connect(this, &NodeControl::selected,
            [this, node]() { node->select(true); });
    connect(runtime, &MaximRuntime::Control::removed,
            this, &NodeControl::remove);
}

std::unique_ptr<NodeControl> NodeControl::fromRuntimeControl(Node *node, size_t index, MaximRuntime::Control *runtime) {
    switch (runtime->type()->type()) {
        case MaximCommon::ControlType::NUMBER:
            return std::make_unique<NodeNumControl>(node, index, runtime, QPoint(0, 0), QSize(2, 2));
        case MaximCommon::ControlType::MIDI:
            return std::make_unique<NodeMidiControl>(node, index, runtime, QPoint(0, 0), QSize(2, 2));
        case MaximCommon::ControlType::NUM_EXTRACT:
            return std::make_unique<NodeExtractControl>(node, index, runtime, ConnectionSink::Type::NUMBER, QPoint(0, 0), QSize(2, 2));
        case MaximCommon::ControlType::MIDI_EXTRACT:
            return std::make_unique<NodeExtractControl>(node, index, runtime, ConnectionSink::Type::MIDI, QPoint(0, 0), QSize(2, 2));
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

void NodeControl::setExposeBase(AxiomModel::NodeControl *base) {
    if (node->parentSchematic->canExposeControl() && base != _exposeBase) {
        _exposeBase = base;
        emit exposeBaseChanged(base);
    }
}

void NodeControl::serialize(QDataStream &stream) const {
    GridItem::serialize(stream);

    stream << m_showName;
    // todo: control exposing
}

void NodeControl::deserialize(QDataStream &stream) {
    GridItem::deserialize(stream);

    bool showName; stream >> showName;
    setShowName(showName);
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
