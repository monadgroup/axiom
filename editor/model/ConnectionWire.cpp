#include "ConnectionWire.h"

#include "grid/GridSurface.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(const AxiomModel::GridSurface *grid, WireType wireType, const QPointF &startPos, const QPointF &endPos)
    : _grid(grid), _wireType(wireType), _startPos(startPos), _endPos(endPos) {
    updateRoute();
}

void ConnectionWire::setStartPos(const QPointF &startPos) {
    if (startPos != _startPos) {
        _startPos = startPos;
        startPosChanged.trigger(startPos);
        updateRoute();
    }
}

void ConnectionWire::setEndPos(const QPointF &endPos) {
    if (endPos != _endPos) {
        _endPos = endPos;
        endPosChanged.trigger(endPos);
        updateRoute();
    }
}

void ConnectionWire::setStartActive(bool active) {
    if (active != _startActive) {
        _startActive = active;
        updateActive();
    }
}

void ConnectionWire::setEndActive(bool active) {
    if (active != _endActive) {
        _endActive = active;
        updateActive();
    }
}

void ConnectionWire::remove() {
    removed.trigger();
}

void ConnectionWire::updateActive() {
    if (activeState == ActiveState::NONE) {
        activeState = _startActive ? ActiveState::START :
                      _endActive ? ActiveState::END :
                      ActiveState::NONE;
    }

    auto newActive = false;
    switch (activeState) {
        case ActiveState::START:
            newActive = _startActive;
            break;
        case ActiveState::END:
            newActive = _endActive;
            break;
        default:
            break;
    }

    if (newActive != _active) {
        _active = newActive;
        activeChanged.trigger(newActive);
    }

    if (!_startActive && !_endActive) {
        activeState = ActiveState::NONE;
    }
}

void ConnectionWire::updateRoute() {
    _route = _grid->grid().findPath(_startPos.toPoint(), _endPos.toPoint(), 1, 10, 4);
    routeChanged.trigger(_route);
}
