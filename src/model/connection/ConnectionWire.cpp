#include "ConnectionWire.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(Schematic *schematic, ConnectionSource *source, ConnectionSink *sink)
        : schematic(schematic), source(source), sink(sink) {
    updateRoute();
}

void ConnectionWire::updateRoute() {

}

void ConnectionWire::remove() {
    emit removed();
}
