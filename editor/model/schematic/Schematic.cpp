#include "Schematic.h"

#include <cassert>
#include <iostream>
#include <QtCore/QBuffer>

#include "../Project.h"
#include "../node/GroupNode.h"
#include "../node/CustomNode.h"
#include "../node/IONode.h"
#include "../control/NodeControl.h"
#include "../history/CreateNodeOperation.h"
#include "../history/ConnectControlsOperation.h"
#include "../history/AddNodeOperation.h"
#include "../../AxiomApplication.h"
#include "../../util.h"
#include "compiler/runtime/Runtime.h"

struct SinkIndex {
    uint32_t nodeIndex = 0;
    uint32_t controlIndex = 0;
};

static QDataStream &operator<<(QDataStream &stream, const SinkIndex &index) {
    stream << index.nodeIndex;
    stream << index.controlIndex;
    return stream;
}

static QDataStream &operator>>(QDataStream &stream, SinkIndex &index) {
    stream >> index.nodeIndex;
    stream >> index.controlIndex;
    return stream;
}

using namespace AxiomModel;

Schematic::Schematic(Project *project) : _project(project) {

}

void Schematic::attachRuntime(MaximRuntime::Surface *runtime) {
    assert(!_runtime);
    _runtime = runtime;

    // todo: attach nodes that already exist in the runtime to nodes here
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

/*GroupNode *Schematic::groupSelection() {
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

    return modulePtr;

    unreachable;
}*/

void Schematic::partialSerialize(QDataStream &stream, const std::vector<GridItem*> &items, QPoint center) {
    // Later on, we only want to serialize connections that connect between two of the items passed.
    // In order to do this efficiently, we use two maps: firstConnections and secondConnections
    // When a ConnectionWire is first encountered (that is, it's not in firstConnections yet) we add it
    // to firstConnections, alongside a node/control index (relative to the items we're actually serializing).
    // If a ConnectionWire is encountered a second time (meaning we've found its other end), it's put
    // in the secondConnections map, alongside the node/control index of the other control.
    // This means that, after iterating through all our nodes, secondConnections contains a list of all
    // wires that have both ends attached to an item in this list. We also know the node/control indexes
    // of both ends, which are used to serialize the connections.
    std::unordered_map<ConnectionWire*, SinkIndex> firstConnections;
    std::map<ConnectionWire*, SinkIndex> secondConnections;

    // serialize nodes
    stream << (uint32_t) items.size();
    for (size_t nodeI = 0; nodeI < items.size(); nodeI++) {
        auto node = dynamic_cast<Node*>(items[nodeI]);
        assert(node);
        stream << (uint8_t) node->type;

        // serialize into a buffer so deserialization knows size
        QByteArray serializeArray;
        QDataStream serializeStream(&serializeArray, QIODevice::WriteOnly);
        node->serialize(serializeStream, -center);
        stream << serializeArray;

        for (size_t controlI = 0; controlI < node->surface.items().size(); controlI++) {
            auto control = dynamic_cast<NodeControl*>(node->surface.items()[controlI].get());
            assert(control);

            for (const auto &connection : control->sink()->connections()) {
                if (firstConnections.find(connection) == firstConnections.end()) {
                    firstConnections.emplace(connection, SinkIndex {(uint32_t) nodeI, (uint32_t) controlI});
                } else {
                    secondConnections.emplace(connection, SinkIndex {(uint32_t) nodeI, (uint32_t) controlI});
                }
            }
        }
    }

    // serialize connections
    stream << (uint32_t) secondConnections.size();
    for (const auto &pair : secondConnections) {
        auto firstConnectionIndex = firstConnections.find(pair.first);
        assert(firstConnectionIndex != firstConnections.end());

        stream << firstConnectionIndex->second;
        stream << pair.second;
    }
}

void Schematic::partialDeserialize(QDataStream &stream, QPoint center) {
    auto nodeStart = items().size();

    uint32_t nodeCount;
    stream >> nodeCount;
    for (uint32_t i = 0; i < nodeCount; i++) {
        uint8_t intType;
        stream >> intType;

        auto type = (Node::Type) intType;
        if (type == Node::Type::IO) {
            // skip the size
            quint32 notUsed;
            stream >> notUsed;

            assert(items().size() > i);
            nodeStart--;
            auto ioItem = dynamic_cast<IONode *>(items()[i].get());
            assert(ioItem);
            ioItem->deserialize(stream, center);
        } else {
            // read into a byte array to pass into the operation
            QByteArray readArray;
            stream >> readArray;

            _project->history.appendOperation(std::make_unique<AddNodeOperation>(
                _project, NodeRef(ref(), nodeStart + i), type, center, std::move(readArray)
            ));
        }
    }

    uint32_t wireCount;
    stream >> wireCount;
    for (uint32_t i = 0; i < wireCount; i++) {
        SinkIndex firstIndex, secondIndex;

        stream >> firstIndex;
        stream >> secondIndex;

        // serialized indexes are relative to the nodes we're deserializing
        firstIndex.nodeIndex += nodeStart;
        secondIndex.nodeIndex += nodeStart;

        assert(firstIndex.nodeIndex < items().size() && secondIndex.nodeIndex < items().size());
        auto firstNode = dynamic_cast<Node *>(items()[firstIndex.nodeIndex].get());
        auto secondNode = dynamic_cast<Node *>(items()[secondIndex.nodeIndex].get());
        assert(firstNode && secondNode);

        assert(firstIndex.controlIndex < firstNode->surface.items().size()
               && secondIndex.controlIndex < secondNode->surface.items().size());
        auto firstControl = dynamic_cast<NodeControl *>(firstNode->surface.items()[firstIndex.controlIndex].get());
        auto secondControl = dynamic_cast<NodeControl *>(secondNode->surface.items()[secondIndex.controlIndex].get());
        assert(firstControl && secondControl);

        connectControls(firstControl, secondControl);
    }
}

void Schematic::copyIntoSelf(const std::vector<AxiomModel::GridItem *> &items, QPoint center) {
    QPoint currentCenter = findCenter(items);
    std::cout << "Mapping items from " << currentCenter.x() << "," << currentCenter.y() << " to " << center.x() << "," << center.y() << std::endl;

    // first serialize items into a buffer
    QByteArray serializedBuffer;
    QDataStream serializeStream(&serializedBuffer, QIODevice::WriteOnly);
    partialSerialize(serializeStream, items, currentCenter);

    // now deserialize onto us
    QDataStream deserializeStream(&serializedBuffer, QIODevice::ReadOnly);
    partialDeserialize(deserializeStream, center);
}

void Schematic::addWire(std::unique_ptr<ConnectionWire> wire) {
    auto ptr = wire.get();
    m_wires.push_back(std::move(wire));

    connect(ptr, &ConnectionWire::cleanup,
            this, [this, ptr]() { removeWire(ptr); });

    emit wireAdded(ptr);
}

Node *Schematic::addFromStream(AxiomModel::Node::Type type, size_t index, QDataStream &stream, QPoint center) {
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

    newNode->deserialize(stream, center);

    // create and attach runtime
    if (_runtime) {
        newNode->createAndAttachRuntime(_runtime);
    }

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

void Schematic::serialize(QDataStream &stream) const {
    stream << pan();
    stream << zoom();

    partialSerialize(stream, rawItems(), QPoint(0, 0));
}

void Schematic::deserialize(QDataStream &stream) {
    QPointF pan;
    stream >> pan;
    float zoom;
    stream >> zoom;
    setPan(pan);
    setZoom(zoom);

    partialDeserialize(stream, QPoint(0, 0));
}

void Schematic::addNode(Node::Type type, QString name, QPoint pos) {
    _project->history.appendOperation(
        std::make_unique<CreateNodeOperation>(_project, NodeRef(ref(), items().size()), type, name, pos));
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
