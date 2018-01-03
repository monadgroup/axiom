#include "ConnectionWire.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(ConnectionSource *source, ConnectionSink *sink) : source(source), sink(sink) {
    updateRoute();
}

void ConnectionWire::updateRoute() {

}

void ConnectionWire::remove() {
    emit removed();
}
