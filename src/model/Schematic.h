#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointF>

#include "Node.h"

namespace std {
    template<>
    struct hash<QPoint> {
        std::size_t operator()(const QPoint &p) const {
            return std::hash<int>()(p.x()) ^ (std::hash<int>()(p.y()) >> 1);
        }
    };
}

namespace AxiomModel {

    class Schematic : public QObject {
    Q_OBJECT

    public:
        virtual QString name() = 0;

        QPointF pan() const { return m_pan; }

        std::vector<std::unique_ptr<Node>> &nodes() { return m_nodes; }

        bool hasSelection() { return !selectedNodes.empty(); }

        virtual void serialize(QDataStream &stream) const;

        virtual void deserialize(QDataStream &stream);

        void addNode(std::unique_ptr<Node> node);

        void deleteSelectedNodes();

        bool positionAvailable(QPoint pos, QSize size, const Node *ignore = nullptr) const;

        QPoint findNearestPos(QPoint pos, QSize size, const Node *ignore = nullptr) const;

        void freeGridRect(QPoint pos, QSize size);

        void setGridRect(QPoint pos, QSize size, const Node *node);

    public slots:

        void setPan(QPointF pan);

        void selectAll();

        void deselectAll();

    private slots:

        void removeNode(Node *node);

        void selectNode(Node *node, bool exclusive);

        void deselectNode(Node *node);

        void startDragging();

        void dragTo(QPoint delta);

        void finishDragging();

    signals:

        void panChanged(QPointF newPan);

        void nodeAdded(Node *node);

    private:
        QPointF m_pan;
        std::vector<std::unique_ptr<Node>> m_nodes;
        std::vector<Node *> selectedNodes;
        std::unordered_map<QPoint, const Node *> grid;

        QPoint lastDragDelta;

        bool isAllDragAvailable(QPoint delta);

        QPoint findAvailableDelta(QPoint delta);
    };

}
