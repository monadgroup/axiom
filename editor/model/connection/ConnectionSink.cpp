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
    if (pos != m_pos || subPos != m_subPos) {
        m_pos = pos;
        m_subPos = subPos;
        emit posChanged(pos);
    }
}

void ConnectionSink::setActive(bool active) {
    if (active != m_active) {
        m_active = active;
        emit activeChanged(active);
    }
}

void ConnectionSink::removeWire(ConnectionWire *wire) {
    auto loc = std::find(m_connections.begin(), m_connections.end(), wire);
    if (loc != m_connections.end()) {
        m_connections.erase(loc);
        emit connectionRemoved(wire);
    }
}
