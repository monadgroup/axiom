#include "ConnectionWire.h"

#include "ConnectionSink.h"
#include "../GridSurface.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(GridSurface *surface, ConnectionSink *sinkA, ConnectionSink *sinkB)
        : surface(surface), sinkA(sinkA), sinkB(sinkB) {
    connect(sinkA, &ConnectionSink::posChanged,
            this, &ConnectionWire::updateRoute);
    connect(sinkB, &ConnectionSink::posChanged,
            this, &ConnectionWire::updateRoute);

    updateRoute();
}

void ConnectionWire::remove() {
    emit removed();
    emit cleanup();
}

void ConnectionWire::updateRoute() {
    m_route = surface->grid.findPath(sinkA->pos(), sinkB->pos(), 0, 100, 10);
    emit routeChanged(m_route);
}
