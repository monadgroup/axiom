#include "ConnectionSink.h"

#include "ConnectionWire.h"

using namespace AxiomModel;

void ConnectionSink::addWire(ConnectionWire *wire) {
    m_connections.push_back(wire);

    connect(wire, &ConnectionWire::removed,
            this, [this, wire]() { removeWire(wire); });

    emit connectionAdded(wire);
}

void ConnectionSink::setPos(QPoint pos, QPointF subPos) {
    if (pos != m_pos) {
        m_pos = pos;
        emit posChanged(pos);
    }
    if (subPos != m_subPos) {
        m_subPos = subPos;
        emit subPosChanged(subPos);
    }
}

void ConnectionSink::setActive(bool active) {
    if (active != m_active) {
        m_active = active;
        emit activeChanged(active);
    }
}

void ConnectionSink::clearConnections() {
    while (!m_connections.empty()) {
        m_connections[0]->remove();
    }
}

void ConnectionSink::removeWire(ConnectionWire *wire) {
    auto loc = std::find(m_connections.begin(), m_connections.end(), wire);
    if (loc != m_connections.end()) {
        m_connections.erase(loc);
        emit connectionRemoved(wire);
    }
}