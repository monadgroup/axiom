#include "ConnectionWire.h"

#include <cassert>

#include "../schematic/Schematic.h"
#include "../Project.h"
#include "../history/DisconnectControlsOperation.h"
#include "compiler/runtime/Control.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(Schematic *schematic, ConnectionSink *sinkA, ConnectionSink *sinkB)
    : schematic(schematic), sinkA(sinkA), sinkB(sinkB), type(sinkA->type) {
    assert(sinkA->type == sinkB->type);

    connect(schematic, &Schematic::gridChanged,
            this, &ConnectionWire::updateRoute);
    connect(sinkA, &ConnectionSink::posChanged,
            this, &ConnectionWire::updateRoute);
    connect(sinkB, &ConnectionSink::posChanged,
            this, &ConnectionWire::updateRoute);
    connect(sinkA, &ConnectionSink::subPosChanged,
            this, &ConnectionWire::subPosChanged);
    connect(sinkB, &ConnectionSink::subPosChanged,
            this, &ConnectionWire::subPosChanged);
    connect(sinkA, &ConnectionSink::activeChanged,
            this, &ConnectionWire::updateActive);
    connect(sinkB, &ConnectionSink::activeChanged,
            this, &ConnectionWire::updateActive);
    connect(this, &ConnectionWire::activeChanged,
            sinkA, &ConnectionSink::setActive);
    connect(this, &ConnectionWire::activeChanged,
            sinkB, &ConnectionSink::setActive);
    connect(sinkA, &ConnectionSink::removed,
            this, &ConnectionWire::remove);
    connect(sinkB, &ConnectionSink::removed,
            this, &ConnectionWire::remove);

    sinkA->addWire(this);
    sinkB->addWire(this);

    if (sinkA->runtime() && sinkB->runtime()) {
        sinkA->runtime()->connectTo(sinkB->runtime());
    }

    updateRoute();
    updateActive();
}

void ConnectionWire::remove() {
    // todo: create op

    schematic->project()->history.appendOperation(std::make_unique<DisconnectControlsOperation>(schematic->project(), sinkA->ref(), sinkB->ref()));
}

void ConnectionWire::removeNoOp() {
    if (sinkA->runtime() && sinkB->runtime()) {
        sinkA->runtime()->disconnectFrom(sinkB->runtime());
    }

    emit removed();
    emit cleanup();
}

void ConnectionWire::updateRoute() {
    m_route = schematic->grid.findPath(sinkA->pos(), sinkB->pos(), 1, 10, 4);
    emit routeChanged(m_route);
}

void ConnectionWire::updateActive() {
    if (activeState == ActiveState::NOTHING) {
        activeState = sinkA->active() ? ActiveState::SINK_A :
                      sinkB->active() ? ActiveState::SINK_B :
                      ActiveState::NOTHING;
    }

    auto newActive = false;
    switch (activeState) {
        case ActiveState::SINK_A:
            newActive = sinkA->active();
            break;
        case ActiveState::SINK_B:
            newActive = sinkB->active();
            break;
        default:
            break;
    }

    if (newActive != m_active) {
        m_active = newActive;
        emit activeChanged(m_active);
    }

    if (!sinkA->active() && !sinkB->active()) {
        activeState = ActiveState::NOTHING;
    }
}
