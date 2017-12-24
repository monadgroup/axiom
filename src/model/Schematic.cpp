#include "Schematic.h"

#include <QDataStream>
#include <cmath>
#include <set>
#include <unordered_set>

using namespace AxiomModel;

void Schematic::serialize(QDataStream &stream) const {
    stream << pan() << static_cast<quint32>(m_nodes.size());
    for (const auto &node : m_nodes) {
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

void Schematic::addNode(std::unique_ptr<Node> node) {
    auto ptr = node.get();
    m_nodes.push_back(std::move(node));

    connect(ptr, &Node::removed,
            this, [this, ptr]() { removeNode(ptr); });
    connect(ptr, &Node::selected,
            this, [this, ptr](bool exclusive) { selectNode(ptr, exclusive); });
    connect(ptr, &Node::deselected,
            this, [this, ptr]() { deselectNode(ptr); });

    connect(ptr, &Node::startedDragging,
            this, &Schematic::startDragging);
    connect(ptr, &Node::draggedTo,
            this, &Schematic::dragTo);
    connect(ptr, &Node::finishedDragging,
            this, &Schematic::finishDragging);

    emit nodeAdded(ptr);
}

void Schematic::deleteSelectedNodes() {
    while (!selectedNodes.empty()) {
        selectedNodes[0]->remove();
    }
}

bool Schematic::positionAvailable(QPoint pos, QSize *size, const Node *ignore) const {
    for (auto dx = 0; dx < size->width(); dx++) {
        for (auto dy = 0; dy < size->height(); dy++) {
            auto checkP = pos + QPoint(dx, dy);
            auto found = grid.find(checkP);
            if (found != grid.end() && found->second != ignore) {
                // figure out two potential rectangles that fit
                auto rectA = QSize(size->width(), dy);
                auto rectB = QSize(dx, size->height());

                *size = (rectA.width() * rectA.height() > rectB.width() * rectB.height()) ? rectA : rectB;
                return false;
            }
        }
    }
    return true;
}

bool Schematic::positionAvailable(QPoint pos, QSize size, const Node *ignore) const {
    return positionAvailable(pos, &size, ignore);
}

class SortedPos {
public:
    QPoint checkPos;
    QPoint basePos;

    SortedPos(QPoint checkPos, QPoint basePos) : checkPos(checkPos), basePos(basePos) { }

    float dist() const {
        return std::sqrt((float)checkPos.x() * checkPos.x() + checkPos.y() * checkPos.y());
    }

    bool operator<(const SortedPos &other) const {
        return dist() < other.dist();
    }
};

QPoint Schematic::findNearestPos(QPoint pos, QSize size, const Node *ignore) const {
    // breadth-first search to find nearest free area
    std::set<SortedPos> positionQueue;
    std::unordered_set<QPoint> visitedQueue;

    positionQueue.insert(SortedPos(pos, pos));
    visitedQueue.insert(pos);
    while (!positionQueue.empty()) {
        auto oldQueue = std::move(positionQueue);
        positionQueue = std::set<SortedPos>();

        for (const auto &p : oldQueue) {
            if (positionAvailable(p.checkPos, size, ignore)) return p.checkPos;

            QPoint offsets[] = { QPoint(1, 0), QPoint(-1, 0), QPoint(0, -1), QPoint(0, 1) };
            for (const auto &offset : offsets) {
                auto newP = p.checkPos + offset;
                if (visitedQueue.find(newP) == visitedQueue.end()) {
                    positionQueue.insert(SortedPos(newP, pos));
                    visitedQueue.insert(newP);
                }
            }
        }
    }

    // oh no! this should never happen :'(
    return pos;
}

void Schematic::freeGridRect(QPoint pos, QSize size) {
    for (auto dx = 0; dx < size.width(); dx++) {
        for (auto dy = 0; dy < size.height(); dy++) {
            grid.erase(pos + QPoint(dx, dy));
        }
    }
}

void Schematic::setGridRect(QPoint pos, QSize size, const Node *node) {
    for (auto dx = 0; dx < size.width(); dx++) {
        for (auto dy = 0; dy < size.height(); dy++) {
            grid.insert(std::pair<QPoint, const Node*>(pos + QPoint(dx, dy), node));
        }
    }
}

void Schematic::setPan(QPointF pan) {
    if (pan != m_pan) {
        m_pan = pan;
        emit panChanged(pan);
    }
}

void Schematic::selectAll() {
    for (const auto &node : m_nodes) {
        node->select(false);
    }
}

void Schematic::deselectAll() {
    for (const auto &node : m_nodes) {
        node->deselect();
    }
}

void Schematic::removeNode(Node *node) {
    selectedNodes.erase(std::find(selectedNodes.begin(), selectedNodes.end(), node));

    for (auto i = m_nodes.begin(); i < m_nodes.end(); i++) {
        if (i->get() == node) {
            m_nodes.erase(i);
            break;
        }
    }
}

void Schematic::selectNode(Node *node, bool exclusive) {
    if (exclusive) {
        for (const auto &iNode : m_nodes) {
            if (iNode.get() != node) iNode->deselect();
        }
    }
    selectedNodes.push_back(node);
}

void Schematic::deselectNode(Node *node) {
    selectedNodes.erase(std::find(selectedNodes.begin(), selectedNodes.end(), node));
}

void Schematic::startDragging() {
    lastDragDelta = QPoint(0, 0);
    for (auto &node : selectedNodes) {
        node->startDragging();
    }
}

void Schematic::dragTo(QPoint delta) {
    for (auto &node : selectedNodes) {
        freeGridRect(node->pos(), node->size());
    }

    auto availableDelta = findAvailableDelta(delta);
    lastDragDelta = availableDelta;
    for (auto &node : selectedNodes) {
        node->dragTo(availableDelta);
    }

    for (auto &node : selectedNodes) {
        setGridRect(node->pos(), node->size(), node);
    }
}

void Schematic::finishDragging() {
    for (auto &node : selectedNodes) {
        node->finishDragging();
    }
}

bool Schematic::isAllDragAvailable(QPoint delta) {
    for (auto &node : selectedNodes) {
        if (!node->isDragAvailable(delta)) return false;
    }
    return true;
}

QPoint Schematic::findAvailableDelta(QPoint delta) {
    if (isAllDragAvailable(delta)) return delta;
    auto xDelta = QPoint(delta.x(), lastDragDelta.y());
    if (isAllDragAvailable(xDelta)) return xDelta;
    auto yDelta = QPoint(lastDragDelta.x(), delta.y());
    if (isAllDragAvailable(yDelta)) return yDelta;
    return {0, 0};
}
