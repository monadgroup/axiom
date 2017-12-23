#include "Schematic.h"
#include <QDataStream>

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
    m_nodes.push_back(std::move(node));
    emit nodeAdded(ptr);
}

void Schematic::removeNode(Node *node) {
    for (auto it = m_nodes.begin(); it != m_nodes.end(); it++) {
        if (it->get() != node) continue;

        m_nodes.erase(it);
        emit nodeRemoved(node);
        return;
    }
}
