#pragma once
#include <memory>
#include <vector>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointF>

#include "Node.h"

namespace AxiomModel {

    class Schematic : public QObject {
    Q_OBJECT

    public:
        virtual QString name() = 0;

        QPointF pan() const { return m_pan; }
        std::vector<std::unique_ptr<Node>> &nodes() { return m_nodes; }

        virtual void serialize(QDataStream &stream) const;
        virtual void deserialize(QDataStream &stream);

        void addNode(std::unique_ptr<Node> node);

    public slots:
        void setPan(QPointF pan);
        void removeNode(Node *node);

    signals:
        void panChanged(QPointF newPan);
        void nodeAdded(Node *node);
        void nodeRemoved(Node *node);

    private:
        QPointF m_pan;
        std::vector<std::unique_ptr<Node>> m_nodes;
    };

}
