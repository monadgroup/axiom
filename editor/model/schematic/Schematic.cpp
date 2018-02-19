#include "Schematic.h"

#include <QDataStream>
#include <cassert>
#include <iostream>

#include "../node/Node.h"
#include "../node/ModuleNode.h"
#include "../control/NodeControl.h"
#include "editor/AxiomApplication.h"
#include "compiler/runtime/SoftControl.h"

using namespace AxiomModel;

Schematic::Schematic(MaximRuntime::Schematic *runtime) : _runtime(runtime) {

}

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

    // calculate center point for all items
    for (const auto &gridItem : selectedItems()) {
        summedPoints =
                summedPoints + gridItem->pos() + QPoint(gridItem->size().width() / 2, gridItem->size().height() / 2);
    }
    QPoint centerPoint = summedPoints / selectedItems().size();

    // todo: calculate required size for all controls we need to make shared to keep connections working
    QSize surfaceSize(2, 2);

    auto moduleNode = std::make_unique<ModuleNode>(this, tr("New Group"), centerPoint - QPoint(surfaceSize.width() / 2,
                                                                                               surfaceSize.height() /
                                                                                               2), surfaceSize);

    // store some mapping data for updating connections
    std::unordered_map<NodeControl *, NodeControl *> oldControlToNewControlMap;
    std::unordered_map<ConnectionSink *, NodeControl *> oldSinkToNewControlMap;

    // add cloned items to new schematic
    for (const auto &gridItem : selectedItems()) {
        auto newPos = gridItem->pos() - centerPoint;

        auto clonedItem = gridItem->clone(moduleNode->schematic.get(), newPos, gridItem->size());

        if (auto node = dynamic_cast<Node *>(gridItem)) {
            auto clonedNode = dynamic_cast<Node *>(clonedItem.get());
            assert(clonedNode != nullptr);

            assert(clonedNode->surface.items().size() == node->surface.items().size());

            for (auto i = 0; i < node->surface.items().size(); i++) {
                auto oldControl = dynamic_cast<NodeControl *>(node->surface.items()[i].get());
                if (oldControl == nullptr) continue;
                auto newControl = dynamic_cast<NodeControl *>(clonedNode->surface.items()[i].get());
                assert(newControl != nullptr);

                oldSinkToNewControlMap.emplace(oldControl->sink(), newControl);
                oldControlToNewControlMap.emplace(oldControl, newControl);
            }
        }

        moduleNode->schematic->addItem(std::move(clonedItem));
    }

    // update connections
    for (const auto &pair : oldControlToNewControlMap) {
        auto oldControl = pair.first;
        auto newControl = pair.second;

        for (const auto &connection : oldControl->sink()->connections()) {
            // only connect from one side, so we don't get two wires
            // todo: this will need to be changed when we account for external connections too
            if (connection->sinkB == oldControl->sink()) continue;

            auto targetControl = oldSinkToNewControlMap.find(connection->sinkB);
            if (targetControl == oldSinkToNewControlMap.end()) {
                // the connection goes outside the selection
                // todo: forward control to moduleNode's surface and connect things there
                continue;
            }

            moduleNode->schematic->connectSinks(newControl->sink(), targetControl->second->sink());

        }
    }

    // delete old items
    while (hasSelection()) {
        selectedItems()[0]->remove();
    }

    // add new module node to surface
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
    if (sinkA->type != sinkB->type) {
        return nullptr;
    }

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
