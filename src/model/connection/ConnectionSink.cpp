#include "ConnectionSink.h"

#include "ConnectionWire.h"

using namespace AxiomModel;

void ConnectionSink::addWire(ConnectionWire *wire) {
    m_inputs.push_back(wire);

    connect(wire, &ConnectionWire::removed,
            this, [this, wire]() { removeWire(wire); });

    emit inputAdded(wire);
}

void ConnectionSink::setPos(QPoint pos) {
    if (pos != m_pos) {
        m_pos = pos;
        emit posChanged(pos);
    }
}

void ConnectionSink::removeWire(ConnectionWire *wire) {
    auto loc = std::find(m_inputs.begin(), m_inputs.end(), wire);
    if (loc != m_inputs.end()) {
        m_inputs.erase(loc);
    }
}
