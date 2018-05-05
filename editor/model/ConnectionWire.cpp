#include "ConnectionWire.h"

#include "grid/GridSurface.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(const AxiomModel::GridSurface *grid, const QPoint &startPos, const QPoint &endPos)
    : _grid(grid), _startPos(startPos), _endPos(endPos) {
    updateRoute();
}

void ConnectionWire::setStartPos(const QPoint &startPos) {
    if (startPos != _startPos) {
        _startPos = startPos;
        startPosChanged.trigger(startPos);
        updateRoute();
    }
}

void ConnectionWire::setEndPos(const QPoint &endPos) {
    if (endPos != _endPos) {
        _endPos = endPos;
        endPosChanged.trigger(endPos);
        updateRoute();
    }
}

void ConnectionWire::updateRoute() {
    _route = _grid->grid().findPath(_startPos, _endPos, 1, 10, 4);
    routeChanged.trigger(_route);
}
