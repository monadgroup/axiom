#include "GraphControl.h"

using namespace AxiomModel;

GraphControl::GraphControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                           QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                           std::unique_ptr<GraphControlState> savedState, AxiomModel::ModelRoot *root)
    : Control(ControlType::GRAPH, ConnectionWire::WireType::NUM, uuid, parentUuid, pos, size, selected, std::move(name),
              showName, exposerUuid, exposingUuid, root),
      _savedState(std::move(savedState)) {}

std::unique_ptr<GraphControl> GraphControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                   const QUuid &exposingUuid,
                                                   std::unique_ptr<GraphControlState> savedState,
                                                   AxiomModel::ModelRoot *root) {
    return std::make_unique<GraphControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                          exposingUuid, std::move(savedState), root);
}

void GraphControl::doRuntimeUpdate() {
    // hash the current state so we can compare it
    auto currentState = state();
    size_t newStateHash = 0;
    if (currentState) {
        newStateHash = 17;
        newStateHash = newStateHash * 31 + std::hash<uint8_t>()(currentState->curveCount);
        newStateHash = newStateHash * 31 + std::hash<float>()(currentState->curveStartVals[0]);
        for (uint8_t curveIndex = 0; curveIndex < currentState->curveCount; curveIndex++) {
            newStateHash = newStateHash * 31 + std::hash<float>()(currentState->curveStartVals[curveIndex + 1]);
            newStateHash = newStateHash * 31 + std::hash<float>()(currentState->curveEndPositions[curveIndex]);
            newStateHash = newStateHash * 31 + std::hash<float>()(currentState->curveTension[curveIndex]);
            newStateHash = newStateHash * 31 + std::hash<int8_t>()(currentState->curveStates[curveIndex]);
        }
    }

    if (newStateHash != _lastStateHash) {
        stateChanged.trigger();
        _lastStateHash = newStateHash;
    }
}

GraphControlState *GraphControl::state() const {
    if (runtimePointers()) {
        return (GraphControlState *) runtimePointers()->data;
    } else {
        return nullptr;
    }
}

void GraphControl::setZoom(float zoom) {
    if (zoom != _zoom) {
        _zoom = zoom;
        zoomChanged.trigger(zoom);
    }
}

void GraphControl::setScroll(float scroll) {
    if (scroll != _scroll) {
        _scroll = scroll;
        scrollChanged.trigger(scroll);
    }
}

void GraphControl::insertPoint(float time, float val) {
    auto controlState = state();
    if (!controlState) return;

    // can't place the point if there are too many curves already
    if (controlState->curveCount == GRAPH_CONTROL_CURVE_COUNT) {
        return;
    }

    if (controlState->curveCount == 0 || time >= controlState->curveEndPositions[controlState->curveCount - 1]) {
        // inherit tension from the previous curve if possible
        auto newTension = controlState->curveCount == 0 ? 0 : controlState->curveTension[controlState->curveCount - 1];

        // if the point is after the last, adding is trivial
        controlState->curveStartVals[controlState->curveCount + 1] = val;
        controlState->curveEndPositions[controlState->curveCount] = time;
        controlState->curveTension[controlState->curveCount] = newTension;
        controlState->curveStates[controlState->curveCount] = -1;
        controlState->curveCount++;
    } else {
        // figure out after which curve the new point should be placed
        ssize_t placePoint = -1;
        for (size_t i = 0; i < controlState->curveCount; i++) {
            if (time < controlState->curveEndPositions[i]) {
                placePoint = i;
                break;
            }
        }
        assert(placePoint != -1);

        auto newTension = controlState->curveTension[placePoint];

        // move old values after the place point
        auto moveItems = controlState->curveCount - placePoint;
        memmove(&controlState->curveStartVals[placePoint + 2], &controlState->curveStartVals[placePoint + 1],
                sizeof(controlState->curveStartVals[0]) * moveItems);
        memmove(&controlState->curveEndPositions[placePoint + 1], &controlState->curveEndPositions[placePoint],
                sizeof(controlState->curveEndPositions[0]) * moveItems);
        memmove(&controlState->curveTension[placePoint + 1], &controlState->curveTension[placePoint],
                sizeof(controlState->curveTension[0]) * moveItems);
        memmove(&controlState->curveStates[placePoint + 1], &controlState->curveStates[placePoint],
                sizeof(controlState->curveStates[0]) * moveItems);

        controlState->curveStartVals[placePoint + 1] = val;
        controlState->curveEndPositions[placePoint] = time;
        controlState->curveTension[placePoint] = newTension;
        controlState->curveStates[placePoint] = -1;
        controlState->curveCount++;
    }
}

void GraphControl::removePoint(uint8_t index) {
    // can't remove the first point ever
    if (index == 0) return;

    auto controlState = state();
    if (!controlState) return;

    // move values to cover up the point
    auto moveItems = controlState->curveCount - index;
    memmove(&controlState->curveStartVals[index], &controlState->curveStartVals[index + 1],
            sizeof(controlState->curveStartVals[0]) * moveItems);
    memmove(&controlState->curveEndPositions[index - 1], &controlState->curveEndPositions[index],
            sizeof(controlState->curveEndPositions[0]) * moveItems);
    memmove(&controlState->curveTension[index - 1], &controlState->curveTension[index],
            sizeof(controlState->curveTension[0]) * moveItems);
    memmove(&controlState->curveStates[index - 1], &controlState->curveStates[index],
            sizeof(controlState->curveStates[0]) * moveItems);
    controlState->curveCount--;
}

void GraphControl::saveState() {
    auto controlState = state();
    if (!controlState) return;

    _savedState = std::make_unique<GraphControlState>();
    memcpy(_savedState.get(), controlState, sizeof(*controlState));
}

void GraphControl::restoreState() {
    auto controlState = state();
    if (!controlState || !_savedState) return;

    memcpy(controlState, _savedState.get(), sizeof(*controlState));
    _savedState.reset();
}
