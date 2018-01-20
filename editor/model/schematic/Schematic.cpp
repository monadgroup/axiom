#include "Schematic.h"

#include <QDataStream>

#include "../node/ModuleNode.h"

using namespace AxiomModel;

void Schematic::serialize(QDataStream &stream) const {
    stream << pan() << static_cast<quint32>(items().size());
    for (const auto &node : items()) {
        //node.serialize(stream);
    }
}

void Schematic::deserialize(QDataStream &stream) {
    QPointF pan;
    quint32 nodeCount;

    stream >> pan >> nodeCount;
    setPan(pan);

    for (auto i = 0; i < nodeCount; i++) {
        // todo
    }
}

void Schematic::setPan(QPointF pan) {
    if (pan != m_pan) {
        m_pan = pan;
        emit panChanged(pan);
    }
}

ModuleNode *Schematic::groupSelection() {
    // todo: this function should create 'shared' controls and maintain outside and inside connections between them,
    // so the resulting connection graph is identical to before the operation

    QPoint summedPoints;
    for (const auto &gridItem : selectedItems()) {
        summedPoints = summedPoints + gridItem->pos() + QPoint(gridItem->size().width() / 2, gridItem->size().height() / 2);
    }
    QPoint centerPoint = summedPoints / selectedItems().size();

    // todo: calculate required size for all controls we need to make shared to keep connections working
    QSize surfaceSize(2, 2);

    auto moduleNode = std::make_unique<ModuleNode>(this, tr("New Group"), centerPoint - QPoint(surfaceSize.width() / 2, surfaceSize.height() / 2), surfaceSize);

    for (const auto &gridItem : selectedItems()) {
        auto newPos = gridItem->pos() - centerPoint;

        auto clonedItem = gridItem->clone(moduleNode->schematic.get(), newPos, gridItem->size());

        // todo: handle connections

        moduleNode->schematic->addItem(std::move(clonedItem));
    }

    while (hasSelection()) {
        selectedItems()[0]->remove();
    }

    auto modulePtr = moduleNode.get();
    addItem(std::move(moduleNode));
    modulePtr->select(true);

    return modulePtr;
}

void Schematic::addWire(std::unique_ptr<ConnectionWire> wire) {
    auto ptr = wire.get();
    m_wires.push_back(std::move(wire));

    connect(ptr, &ConnectionWire::cleanup,
            this, [this, ptr]() { removeWire(ptr); });

    emit wireAdded(ptr);
}

ConnectionWire *Schematic::connectSinks(ConnectionSink *sinkA, ConnectionSink *sinkB) {
    auto wire = std::make_unique<ConnectionWire>(this, sinkA, sinkB);
    auto ptr = wire.get();
    addWire(std::move(wire));
    return ptr;
}

void Schematic::removeWire(ConnectionWire *wire) {
    for (auto i = m_wires.begin(); i < m_wires.end(); i++) {
        if (i->get() == wire) {
            m_wires.erase(i);
            break;
        }
    }
}
