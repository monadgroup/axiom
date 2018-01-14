#include "ConnectionWire.h"

#include "ConnectionSource.h"
#include "ConnectionSink.h"
#include "../GridSurface.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(GridSurface *surface, ConnectionSource *source, ConnectionSink *sink)
        : surface(surface), source(source), sink(sink) {
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
    m_route = surface->grid.findPath(source->pos(), sink->pos(), 0, 100, 10);
    emit routeChanged(m_route);
}
