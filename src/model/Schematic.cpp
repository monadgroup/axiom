#include "Schematic.h"

#include <QDataStream>
#include <cmath>
#include <set>
#include <unordered_set>

using namespace AxiomModel;
void Schematic::setPan(QPointF pan) {
    if (pan != m_pan) {
        m_pan = pan;
        emit panChanged(pan);
    }
}

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
    //node->setPos(findNearestPos(node->pos(), node->size()));
    m_nodes.push_back(std::move(node));
    /*for (auto dx = 0; dx < node->size().width(); dx++) {
        for (auto dy = 0; dy < node->size().height(); dy++) {
            auto insertP = node->pos() + QPoint(dx, dy);
            grid.
        }
    }*/
    emit nodeAdded(ptr);
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

void Schematic::removeNode(Node *node) {
    for (auto it = m_nodes.begin(); it != m_nodes.end(); it++) {
        if (it->get() != node) continue;

        m_nodes.erase(it);
        emit nodeRemoved(node);
        return;
    }
}
