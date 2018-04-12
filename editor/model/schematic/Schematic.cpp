#include "Schematic.h"

#include <cassert>
#include <iostream>
#include <QtCore/QBuffer>

#include "../Project.h"
#include "../node/GroupNode.h"
#include "../node/CustomNode.h"
#include "../node/IONode.h"
#include "../control/NodeControl.h"
#include "../history/AddNodeOperation.h"
#include "../history/ConnectControlsOperation.h"
#include "../../AxiomApplication.h"
#include "../../util.h"
#include "compiler/runtime/Runtime.h"


using namespace AxiomModel;

Schematic::Schematic(Project *project, MaximRuntime::Surface *runtime) : _project(project), _runtime(runtime) {

}

void Schematic::setPan(QPointF pan) {
    if (pan != m_pan) {
        m_pan = pan;
        emit panChanged(pan);
    }
}

void Schematic::setZoom(float zoom) {
    if (zoom != m_zoom) {
        m_zoom = zoom;
        emit zoomChanged(zoom);
    }
}

GroupNode *Schematic::groupSelection() {
    // todo: this function should create 'shared' controls and maintain outside and inside connections between them,
    // so the resulting connection graph is identical to before the operation

    /*QPoint summedPoints;

    // calculate center point for all items
    for (const auto &gridItem : selectedItems()) {
        summedPoints =
            summedPoints + gridItem->pos() + QPoint(gridItem->size().width() / 2, gridItem->size().height() / 2);
    }
    QPoint centerPoint = summedPoints / selectedItems().size();

    // todo: calculate required size for all controls we need to make shared to keep connections working
    QSize surfaceSize(2, 2);

    auto moduleNode = std::make_unique<GroupNode>(this, tr("New Group"), centerPoint - QPoint(surfaceSize.width() / 2,
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

            for (size_t i = 0; i < node->surface.items().size(); i++) {
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

    return modulePtr;*/

    unreachable;
}

void Schematic::addWire(std::unique_ptr<ConnectionWire> wire) {
    auto ptr = wire.get();
    m_wires.push_back(std::move(wire));

    connect(ptr, &ConnectionWire::cleanup,
            this, [this, ptr]() { removeWire(ptr); });

    emit wireAdded(ptr);
}

Node *Schematic::addFromStream(AxiomModel::Node::Type type, size_t index, QDataStream &stream) {
    std::unique_ptr<Node> newNode;
    switch (type) {
        case Node::Type::CUSTOM:
            newNode = std::make_unique<CustomNode>(this, "", QPoint(0, 0), QSize(0, 0));
            break;
        case Node::Type::GROUP:
            newNode = std::make_unique<GroupNode>(this, "", QPoint(0, 0), QSize(0, 0));
            break;
        default:
            break;
    }

    if (!newNode) return nullptr;

    newNode->deserialize(stream);
    auto nodePtr = newNode.get();
    insertItem(index, std::move(newNode));
    return nodePtr;
}

void Schematic::connectControls(AxiomModel::NodeControl *controlA, AxiomModel::NodeControl *controlB) {
    if (controlA->sink()->type != controlB->sink()->type) {
        return;
    }

    _project->history.appendOperation(std::make_unique<ConnectControlsOperation>(_project, controlA->ref(), controlB->ref()));
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

struct SinkIndex {
    uint32_t nodeIndex = 0;
    uint32_t controlIndex = 0;
};

static QDataStream &operator<<(QDataStream &stream, SinkIndex &index) {
    stream << index.nodeIndex;
    stream << index.controlIndex;
    return stream;
}

static QDataStream &operator>>(QDataStream &stream, SinkIndex &index) {
    stream >> index.nodeIndex;
    stream >> index.controlIndex;
    return stream;
}

void Schematic::serialize(QDataStream &stream) const {
    stream << pan();
    stream << zoom();

    // serialize nodes - also build an index of ConnectionSinks to node/control indexes for connection serialization
    stream << (uint32_t) items().size();
    std::unordered_map<ConnectionSink *, SinkIndex> sinkIndexes;
    auto &itms = items();
    for (size_t nodeI = 0; nodeI < itms.size(); nodeI++) {
        auto node = dynamic_cast<Node *>(itms[nodeI].get());
        assert(node);

        // add sinks to index
        auto &nodeItms = node->surface.items();
        for (size_t controlI = 0; controlI < nodeItms.size(); controlI++) {
            if (auto control = dynamic_cast<NodeControl *>(nodeItms[controlI].get())) {
                sinkIndexes.emplace(control->sink(), SinkIndex{(uint32_t) nodeI, (uint32_t) controlI});
            }
        }

        stream << (uint8_t) node->type;

        // serialize the node
        // todo: for now, since IO nodes are skipped, we need to write the size of the node data
        // so we can skip it when deserializing
        QBuffer nodeBuffer;
        nodeBuffer.open(QBuffer::WriteOnly);
        QDataStream dataStream(&nodeBuffer);
        node->serialize(dataStream);

        stream << (quint64) nodeBuffer.size();
        stream.writeRawData(nodeBuffer.data(), (int) nodeBuffer.size());
        nodeBuffer.close();
    }

    // serialize connections using the index built above
    stream << (uint32_t) m_wires.size();
    for (const auto &wire : m_wires) {
        auto firstIter = sinkIndexes.find(wire->sinkA);
        auto secondIter = sinkIndexes.find(wire->sinkB);
        assert(firstIter != sinkIndexes.end() && secondIter != sinkIndexes.end());

        stream << firstIter->second;
        stream << secondIter->second;
    }
}

void Schematic::deserialize(QDataStream &stream) {
    QPointF pan;
    stream >> pan;
    float zoom;
    stream >> zoom;
    setPan(pan);
    setZoom(zoom);

    uint32_t nodeCount;
    stream >> nodeCount;
    for (uint32_t i = 0; i < nodeCount; i++) {
        uint8_t intType;
        stream >> intType;
        quint64 nodeSize;
        stream >> nodeSize;

        auto type = (Node::Type) intType;
        auto added = addFromStream(type, i, stream);
        if (!added) {
            if (type == Node::Type::IO) {
                auto ioItem = dynamic_cast<IONode *>(items()[i].get());
                assert(ioItem);
                ioItem->deserialize(stream);
            } else {
                stream.skipRawData((int) nodeSize);
            }
        }
    }

    auto &itms = items();

    uint32_t wireCount;
    stream >> wireCount;
    for (uint32_t i = 0; i < wireCount; i++) {
        SinkIndex firstIndex, secondIndex;

        stream >> firstIndex;
        stream >> secondIndex;

        assert(firstIndex.nodeIndex < itms.size() && secondIndex.nodeIndex < itms.size());
        auto firstNode = dynamic_cast<Node *>(itms[firstIndex.nodeIndex].get());
        auto secondNode = dynamic_cast<Node *>(itms[secondIndex.nodeIndex].get());
        assert(firstNode && secondNode);

        assert(firstIndex.controlIndex < firstNode->surface.items().size()
               && secondIndex.controlIndex < secondNode->surface.items().size());
        auto firstControl = dynamic_cast<NodeControl *>(firstNode->surface.items()[firstIndex.controlIndex].get());
        auto secondControl = dynamic_cast<NodeControl *>(secondNode->surface.items()[secondIndex.controlIndex].get());
        assert(firstControl && secondControl);

        connectSinks(firstControl->sink(), secondControl->sink());
    }
}

void Schematic::addNode(Node::Type type, QString name, QPoint pos) {
    _project->history.appendOperation(
        std::make_unique<AddNodeOperation>(_project, NodeRef(ref(), items().size()), type, name, pos));
}

void Schematic::remove() {
    for (const auto &item : items()) {
        item->remove();
    }

    emit removed();
    emit cleanup();
}

void Schematic::removeWire(ConnectionWire *wire) {
    for (auto i = m_wires.begin(); i < m_wires.end(); i++) {
        if (i->get() == wire) {
            m_wires.erase(i);
            break;
        }
    }
}
