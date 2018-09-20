#include "ConnectionWire.h"

#include "../util.h"
#include "grid/GridSurface.h"

using namespace AxiomModel;

ConnectionWire::ConnectionWire(AxiomModel::GridSurface *grid, WireGrid *wireGrid, WireType wireType,
                               const QPointF &startPos, const QPointF &endPos)
    : _grid(grid), _wireGrid(wireGrid), _wireType(wireType), _startPos(startPos), _endPos(endPos) {
    _grid->gridChanged.connect(this, &ConnectionWire::updateRoute);
    _wireGrid->gridChanged.connect(this, &ConnectionWire::updateLineIndices);
    updateRoute();
}

ConnectionWire::~ConnectionWire() {
    clearWireGrid(_route);
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
        activeState = _startActive ? ActiveState::START : _endActive ? ActiveState::END : ActiveState::NONE;
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
    clearWireGrid(_route);
    _route = _grid->grid().findPath(QPoint(_startPos.x(), _startPos.y()), QPoint(_endPos.x(), _endPos.y()), 1, 10, 4);
    setWireGrid(_route);
}

void ConnectionWire::updateLineIndices() {
    _lineIndices = getLineIndices(_route);
    routeChanged.trigger(_route, _lineIndices);
}

void ConnectionWire::setWireGrid(const std::deque<QPoint> &route) {
    for (size_t routeIndex = 1; routeIndex < route.size(); routeIndex++) {
        auto lastPoint = route[routeIndex - 1];
        auto currentPoint = route[routeIndex];
        _wireGrid->addRegion(AxiomUtil::makeRect(lastPoint, currentPoint), this);
    }
}

void ConnectionWire::clearWireGrid(const std::deque<QPoint> &route) {
    for (size_t routeIndex = 1; routeIndex < route.size(); routeIndex++) {
        auto lastPoint = route[routeIndex - 1];
        auto currentPoint = route[routeIndex];
        _wireGrid->removeRegion(AxiomUtil::makeRect(lastPoint, currentPoint), this);
    }
}

std::vector<LineIndex> ConnectionWire::getLineIndices(const std::deque<QPoint> &route) {
    std::vector<LineIndex> indices;
    if (route.size() <= 1) return indices;

    indices.reserve(route.size() - 1);

    for (size_t routeIndex = 1; routeIndex < route.size(); routeIndex++) {
        auto lastPoint = route[routeIndex - 1];
        auto currentPoint = route[routeIndex];

        auto regionIndex = _wireGrid->getRegionIndex(AxiomUtil::makeRect(lastPoint, currentPoint), this);
        indices.push_back(regionIndex);
    }

    return indices;
}
