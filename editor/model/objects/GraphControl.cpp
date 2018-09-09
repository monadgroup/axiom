#include "GraphControl.h"

using namespace AxiomModel;

GraphControl::GraphControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                           QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                           std::unique_ptr<GraphControlState> savedState, AxiomModel::ModelRoot *root)
    : Control(ControlType::GRAPH, ConnectionWire::WireType::NUM, QSize(4, 4), uuid, parentUuid, pos, size, selected,
              std::move(name), showName, exposerUuid, exposingUuid, root),
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
        newStateHash = newStateHash * 31 + std::hash<uint8_t>()(currentState->currentState);
        newStateHash = newStateHash * 31 + std::hash<float>()(currentState->curveStartVals[0]);
        newStateHash = newStateHash * 31 + std::hash<uint8_t>()(currentState->curveStates[0]);
        for (uint8_t curveIndex = 0; curveIndex < currentState->curveCount; curveIndex++) {
            newStateHash = newStateHash * 31 + std::hash<float>()(currentState->curveStartVals[curveIndex + 1]);
            newStateHash = newStateHash * 31 + std::hash<float>()(currentState->curveEndPositions[curveIndex]);
            newStateHash = newStateHash * 31 + std::hash<float>()(currentState->curveTension[curveIndex]);
            newStateHash = newStateHash * 31 + std::hash<uint8_t>()(currentState->curveStates[curveIndex + 1]);
        }
    }

    if (newStateHash != _lastStateHash) {
        stateChanged.trigger();
        _lastStateHash = newStateHash;
    }

    if (currentState->currentTimeSamples != _lastTime) {
        timeChanged.trigger();
        _lastTime = currentState->currentTimeSamples;
    }
}

GraphControlState *GraphControl::state() const {
    if (runtimePointers()) {
        return (GraphControlState *) runtimePointers()->data;
    } else {
        return _savedState.get();
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

std::optional<uint8_t> GraphControl::determineInsertIndex(float time) {
    auto controlState = state();

    // can't place the point if there are too many curves already
    if (controlState->curveCount == GRAPH_CONTROL_CURVE_COUNT) {
        return std::nullopt;
    }

    if (controlState->curveCount == 0 || time >= controlState->curveEndPositions[controlState->curveCount - 1]) {
        return controlState->curveCount;
    } else {
        for (size_t i = 0; i < controlState->curveCount; i++) {
            if (time < controlState->curveEndPositions[i]) {
                return i;
            }
        }
        return std::nullopt;
    }
}

void GraphControl::insertPoint(uint8_t index, float time, float val, float tension, uint8_t curveState) {
    auto controlState = state();

    // move old values after the place point
    auto moveItems = controlState->curveCount - index;
    if (moveItems > 0) {
        memmove(&controlState->curveStartVals[index + 2], &controlState->curveStartVals[index + 1],
                sizeof(controlState->curveStartVals[0]) * moveItems);
        memmove(&controlState->curveEndPositions[index + 1], &controlState->curveEndPositions[index],
                sizeof(controlState->curveEndPositions[0]) * moveItems);
        memmove(&controlState->curveTension[index + 1], &controlState->curveTension[index],
                sizeof(controlState->curveTension[0]) * moveItems);
        memmove(&controlState->curveStates[index + 2], &controlState->curveStates[index + 1],
                sizeof(controlState->curveStates[0]) * moveItems);
    }

    controlState->curveStartVals[index + 1] = val;
    controlState->curveEndPositions[index] = time;
    controlState->curveTension[index] = tension;
    controlState->curveStates[index + 1] = curveState;
    controlState->curveCount++;
}

void GraphControl::movePoint(uint8_t index, float time, float value) {
    auto controlState = state();
    controlState->curveStartVals[index] = value;
    if (index > 0) {
        controlState->curveEndPositions[index - 1] = time;
    }
}

void GraphControl::setPointTag(uint8_t index, uint8_t tag) {
    state()->curveStates[index] = tag;
}

void GraphControl::setCurveTension(uint8_t index, float tension) {
    state()->curveTension[index] = tension;
}

void GraphControl::removePoint(uint8_t index) {
    // can't remove the first point ever
    if (index == 0) return;

    auto controlState = state();

    // move values to cover up the point
    auto moveItems = controlState->curveCount - index;
    memmove(&controlState->curveStartVals[index], &controlState->curveStartVals[index + 1],
            sizeof(controlState->curveStartVals[0]) * moveItems);
    memmove(&controlState->curveEndPositions[index - 1], &controlState->curveEndPositions[index],
            sizeof(controlState->curveEndPositions[0]) * moveItems);
    memmove(&controlState->curveTension[index - 1], &controlState->curveTension[index],
            sizeof(controlState->curveTension[0]) * moveItems);
    memmove(&controlState->curveStates[index], &controlState->curveStates[index + 1],
            sizeof(controlState->curveStates[0]) * moveItems);
    controlState->curveCount--;
}

void GraphControl::saveState() {
    if (runtimePointers()) {
        auto controlState = (GraphControlState *) runtimePointers()->data;
        _savedState = std::make_unique<GraphControlState>();
        memcpy(_savedState.get(), controlState, sizeof(*controlState));
    }
}

void GraphControl::restoreState() {
    if (_savedState && runtimePointers()) {
        auto controlState = (GraphControlState *) runtimePointers()->data;
        memcpy(controlState, _savedState.get(), sizeof(*controlState));
        _savedState.reset();
    }
}
