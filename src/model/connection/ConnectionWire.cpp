#include "ConnectionWire.h"

#include "ConnectionSource.h"
#include "ConnectionSink.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(Grid *grid, ConnectionSource *source, ConnectionSink *sink)
        : grid(grid), source(source), sink(sink) {
    connect(source, &ConnectionSource::posChanged,
            this, &ConnectionWire::updateRoute);
    connect(sink, &ConnectionSink::posChanged,
            this, &ConnectionWire::updateRoute);

    updateRoute();
}

void ConnectionWire::remove() {
    emit removed();
}

void ConnectionWire::updateRoute() {
    m_route = grid->findPath(source->pos(), sink->pos(), 0, 100, 10);
    emit routeChanged(m_route);
}
