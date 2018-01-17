#include "Schematic.h"

#include <QDataStream>

using namespace AxiomModel;

void Schematic::serialize(QDataStream &stream) const {
    stream << pan() << static_cast<quint32>(items().size());
    for (const auto &node : items()) {
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

void Schematic::setPan(QPointF pan) {
    if (pan != m_pan) {
        m_pan = pan;
        emit panChanged(pan);
    }
}

void Schematic::addWire(std::unique_ptr<ConnectionWire> wire) {
    auto ptr = wire.get();
    m_wires.push_back(std::move(wire));

    connect(ptr, &ConnectionWire::cleanup,
            this, [this, ptr]() { removeWire(ptr); });

    emit wireAdded(ptr);
}

void Schematic::connectSinks(ConnectionSink *sinkA, ConnectionSink *sinkB) {
    addWire(std::move(std::make_unique<ConnectionWire>(this, sinkA, sinkB)));
}

void Schematic::removeWire(ConnectionWire *wire) {
    for (auto i = m_wires.begin(); i < m_wires.end(); i++) {
        if (i->get() == wire) {
            m_wires.erase(i);
            break;
        }
    }
}
