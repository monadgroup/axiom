#include "ConnectionSource.h"

#include "ConnectionSink.h"
#include "ConnectionWire.h"

using namespace AxiomModel;

ConnectionSource::ConnectionSource(Schematic *schematic) : schematic(schematic) {

}

void ConnectionSource::connectTo(ConnectionSink *sink) {
    auto connection = std::make_unique<ConnectionWire>(schematic, this, sink);
    auto ptr = connection.get();
    m_outputs.push_back(std::move(connection));

    connect(ptr, &ConnectionWire::removed,
            this, [this, ptr]() { removeWire(ptr); });

    sink->addWire(ptr);
    emit outputAdded(ptr);
}

void ConnectionSource::removeWire(ConnectionWire *wire) {
    for (auto i = m_outputs.begin(); i < m_outputs.end(); i++) {
        if (i->get() == wire) {
            m_outputs.erase(i);
            break;
        }
    }
}
