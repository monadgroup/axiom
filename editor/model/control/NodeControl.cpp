#include "NodeControl.h"

#include <cassert>
#include <iostream>

#include "NodeNumControl.h"
#include "NodeMidiControl.h"
#include "NodeExtractControl.h"
#include "../node/GroupNode.h"
#include "../schematic/Schematic.h"
#include "../Project.h"
#include "../history/ShowHideControlNameOperation.h"
#include "../history/MoveControlOperation.h"
#include "../history/SizeControlOperation.h"
#include "compiler/runtime/Control.h"
#include "compiler/codegen/Control.h"
#include "../../util.h"

using namespace AxiomModel;

NodeControl::NodeControl(Node *node, QString name, QPoint pos, QSize size)
    : GridItem(&node->surface, pos, size), node(node) {

    connect(this, &NodeControl::selected,
            this, [node]() { node->select(true); });
}

void NodeControl::attachRuntime(MaximRuntime::Control *runtime) {
    assert(!_runtime);
    _runtime = runtime;

    std::cout << "[NodeControl] attaching runtime" << std::endl;

    connect(runtime, &MaximRuntime::Control::removed,
            this, &NodeControl::remove);

    // connect wires in the runtime
    for (const auto &wire : sink()->connections()) {
        if (wire->sinkA->runtime() && wire->sinkB->runtime()) {
            wire->sinkA->runtime()->connectTo(wire->sinkB->runtime());
        }
    }

    // if I want to be exposed, handle that
    std::cout << "[NodeControl] exposer is " << _exposer << ", parent schematic is " << node->parentSchematic << std::endl;
    if (_exposer) {
        node->parentSchematic->exposeControl(this);
    }
    std::cout << "[NodeControl] finished exposing" << std::endl;
}

std::unique_ptr<NodeControl> NodeControl::create(AxiomModel::Node *node, MaximCommon::ControlType type, QString name) {
    switch (type) {
        case MaximCommon::ControlType::NUMBER:
            return std::make_unique<NodeNumControl>(node, name, QPoint(0, 0), QSize(2, 2));
        case MaximCommon::ControlType::MIDI:
            return std::make_unique<NodeMidiControl>(node, name, QPoint(0, 0), QSize(2, 2));
        case MaximCommon::ControlType::NUM_EXTRACT:
            return std::make_unique<NodeExtractControl>(node, ConnectionSink::Type::NUMBER, name, QPoint(0, 0), QSize(2, 2));
        case MaximCommon::ControlType::MIDI_EXTRACT:
            return std::make_unique<NodeExtractControl>(node, ConnectionSink::Type::MIDI, name, QPoint(0, 0), QSize(2, 2));
        default:
            unreachable;
    }
}

long long int NodeControl::index() const {
    auto &parentItems = node->surface.items();
    auto index = AxiomUtil::findUnique(parentItems.begin(), parentItems.end(), this) - parentItems.begin();
    assert(index >= 0 && index < (long long int) parentItems.size());
    return index;
}

ControlRef NodeControl::ref() const {

    return ControlRef(node->ref(), (size_t) index());
}

std::unique_ptr<GridItem> NodeControl::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    assert(false);
    throw;
}

void NodeControl::setShowName(bool showName) {
    if (showName != m_showName) {
        node->parentSchematic->project()->history.appendOperation(std::make_unique<ShowHideControlNameOperation>(node->parentSchematic->project(), ref(), showName));
    }
}

void NodeControl::setShowNameNoOp(bool showName) {
    if (showName != m_showName) {
        m_showName = showName;
        emit showNameChanged(showName);
    }
}

void NodeControl::setExposer(AxiomModel::NodeControl *base) {
    if (base != _exposer) {
        _exposer = base;
        emit exposerChanged(base);
    }
}

void NodeControl::startResize() {
    startResizeTopLeft = pos();
    startResizeBottomRight = pos() + QPoint(size().width(), size().height());
}

void NodeControl::finishResize() {
    auto bottomRight = pos() + QPoint(size().width(), size().height());
    if (pos() != startResizeTopLeft || bottomRight != startResizeBottomRight) {
        node->parentSchematic->project()->history.appendOperation(std::make_unique<SizeControlOperation>(
            node->parentSchematic->project(),
            ref(),
            startResizeTopLeft, startResizeBottomRight,
            pos(), bottomRight
        ));
    }
}

void NodeControl::startDragging() {
    startDragPos = pos();
    GridItem::startDragging();
}

void NodeControl::finishDragging() {
    GridItem::finishDragging();

    if (startDragPos != pos()) {
        node->parentSchematic->project()->history.appendOperation(std::make_unique<MoveControlOperation>(node->parentSchematic->project(), ref(), startDragPos, pos()));
    }
}

void NodeControl::serialize(QDataStream &stream, QPoint offset) const {
    GridItem::serialize(stream, offset);

    stream << m_showName;

    // serialize control exposing
    stream << (_exposer != nullptr);
    if (_exposer) {
        // we only need to store the exposers index, since we know it belongs to the parent node
        stream << (quint32) _exposer->index();
    }
}

void NodeControl::deserialize(QDataStream &stream, QPoint offset) {
    GridItem::deserialize(stream, offset);

    bool showName;
    stream >> showName;
    setShowNameNoOp(showName);

    bool hasExposer; stream >> hasExposer;
    if (hasExposer) {
        auto parentSurface = dynamic_cast<GroupSchematic *>(node->parentSchematic);
        assert(parentSurface);

        quint32 exposerIndex; stream >> exposerIndex;

        std::cout << "[NodeControl] has exposer at index " << exposerIndex << std::endl;

        auto targetControl = dynamic_cast<NodeControl *>(parentSurface->node->surface.items()[exposerIndex].get());
        setExposer(targetControl);
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
            this, [this](ConnectionWire *wire) {
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
