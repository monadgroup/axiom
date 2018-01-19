#include "ConnectionWire.h"

#include "ConnectionSink.h"
#include "../GridSurface.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(GridSurface *surface, ConnectionSink *sinkA, ConnectionSink *sinkB)
        : surface(surface), sinkA(sinkA), sinkB(sinkB) {
    connect(surface, &GridSurface::gridChanged,
            this, &ConnectionWire::updateRoute);
    connect(sinkA, &ConnectionSink::posChanged,
            this, &ConnectionWire::updateRoute);
    connect(sinkB, &ConnectionSink::posChanged,
            this, &ConnectionWire::updateRoute);
    connect(sinkA, &ConnectionSink::activeChanged,
            this, &ConnectionWire::setActive);
    connect(sinkB, &ConnectionSink::activeChanged,
            this, &ConnectionWire::setActive);
    connect(sinkA, &ConnectionSink::removed,
            this, &ConnectionWire::remove);
    connect(sinkB, &ConnectionSink::removed,
            this, &ConnectionWire::remove);

    updateRoute();
}

void ConnectionWire::remove() {
    emit removed();
    emit cleanup();
}

void ConnectionWire::updateRoute() {
    m_route = surface->grid.findPath(sinkA->pos(), sinkB->pos(), 1, 4, 4);
    emit routeChanged(m_route);
}

void ConnectionWire::setActive(bool active) {
    if (active) {
        activeCount++;
    } else {
        activeCount--;
    }

    if (activeCount < 0) activeCount = 0;

    if (activeCount > 0 && !m_active) {
        m_active = true;
        emit activeChanged(m_active);
    } else if (activeCount == 0 && m_active) {
        m_active = false;
        emit activeChanged(m_active);
    }
}
