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
