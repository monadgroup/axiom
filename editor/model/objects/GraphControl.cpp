#include "GraphControl.h"

using namespace AxiomModel;

GraphControl::GraphControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                           QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                           std::unique_ptr<GraphControlCurveStorage> savedState, AxiomModel::ModelRoot *root)
    : Control(ControlType::GRAPH, ConnectionWire::WireType::NUM, QSize(4, 4), uuid, parentUuid, pos, size, selected,
              std::move(name), showName, exposerUuid, exposingUuid, root) {
    setSavedStorage(std::move(savedState));
}

std::unique_ptr<GraphControl> GraphControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                   const QUuid &exposingUuid,
                                                   std::unique_ptr<GraphControlCurveStorage> savedState,
                                                   AxiomModel::ModelRoot *root) {
    return std::make_unique<GraphControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                          exposingUuid, std::move(savedState), root);
}

QString GraphControl::debugName() {
    return "GraphControl ' " + name() + "'";
}

void GraphControl::doRuntimeUpdate() {
    // hash the current state so we can compare it
    auto currentState = getCurveState();
    size_t newStateHash = 17;
    newStateHash = newStateHash * 31 + std::hash<uint8_t>()(*currentState->curveCount);
    newStateHash = newStateHash * 31 + std::hash<double>()(currentState->curveStartVals[0]);
    newStateHash = newStateHash * 31 + std::hash<uint8_t>()(currentState->curveStates[0]);
    for (uint8_t curveIndex = 0; curveIndex < *currentState->curveCount; curveIndex++) {
        newStateHash = newStateHash * 31 + std::hash<double>()(currentState->curveStartVals[curveIndex + 1]);
        newStateHash = newStateHash * 31 + std::hash<double>()(currentState->curveEndPositions[curveIndex]);
        newStateHash = newStateHash * 31 + std::hash<double>()(currentState->curveTension[curveIndex]);
        newStateHash = newStateHash * 31 + std::hash<uint8_t>()(currentState->curveStates[curveIndex + 1]);
    }

    auto timeState = getTimeState();
    if (timeState) {
        newStateHash = newStateHash * 31 + std::hash<uint8_t>()(timeState->currentState);
    }

    if (newStateHash != _lastStateHash) {
        stateChanged();
        _lastStateHash = newStateHash;
    }

    if (timeState && timeState->currentTimeSamples != _lastTime) {
        _lastTime = timeState->currentTimeSamples;
        timeChanged();
    }
}

GraphControlTimeState *GraphControl::getTimeState() const {
    if (runtimePointers()) {
        return (GraphControlTimeState *) runtimePointers()->data;
    } else {
        return nullptr;
    }
}

GraphControlCurveState *GraphControl::getCurveState() {
    if (runtimePointers()) {
        return (GraphControlCurveState *) runtimePointers()->initialized;
    } else if (_savedStorage) {
        return &_currentState;
    } else {
        return nullptr;
    }
}

void GraphControl::setZoom(float zoom) {
    if (zoom != _zoom) {
        _zoom = zoom;
        zoomChanged(zoom);
    }
}

void GraphControl::setScroll(float scroll) {
    if (scroll != _scroll) {
        _scroll = scroll;
        scrollChanged(scroll);
    }
}

std::optional<uint8_t> GraphControl::determineInsertIndex(double time) {
    auto controlState = getCurveState();

    // can't place the point if there are too many curves already
    if (*controlState->curveCount == GRAPH_CONTROL_CURVE_COUNT) {
        return std::nullopt;
    }

    if (*controlState->curveCount == 0 || time >= controlState->curveEndPositions[*controlState->curveCount - 1]) {
        return *controlState->curveCount;
    } else {
        for (size_t i = 0; i < *controlState->curveCount; i++) {
            if (time < controlState->curveEndPositions[i]) {
                return i;
            }
        }
        return std::nullopt;
    }
}

void GraphControl::insertPoint(uint8_t index, double time, double val, double tension, uint8_t curveState) {
    auto controlState = getCurveState();

    // move old values after the place point
    auto moveItems = *controlState->curveCount - index;
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
    (*controlState->curveCount)++;
}

void GraphControl::movePoint(uint8_t index, double time, double value) {
    auto controlState = getCurveState();
    controlState->curveStartVals[index] = value;
    if (index > 0) {
        controlState->curveEndPositions[index - 1] = time;
    }
}

void GraphControl::setPointTag(uint8_t index, uint8_t tag) {
    getCurveState()->curveStates[index] = tag;
}

void GraphControl::setCurveTension(uint8_t index, double tension) {
    getCurveState()->curveTension[index] = tension;
}

void GraphControl::removePoint(uint8_t index) {
    // can't remove the first point ever
    if (index == 0) return;

    auto controlState = getCurveState();

    // move values to cover up the point
    auto moveItems = *controlState->curveCount - index;
    memmove(&controlState->curveStartVals[index], &controlState->curveStartVals[index + 1],
            sizeof(controlState->curveStartVals[0]) * moveItems);
    memmove(&controlState->curveEndPositions[index - 1], &controlState->curveEndPositions[index],
            sizeof(controlState->curveEndPositions[0]) * moveItems);
    memmove(&controlState->curveTension[index - 1], &controlState->curveTension[index],
            sizeof(controlState->curveTension[0]) * moveItems);
    memmove(&controlState->curveStates[index], &controlState->curveStates[index + 1],
            sizeof(controlState->curveStates[0]) * moveItems);
    (*controlState->curveCount)--;
}

void GraphControl::saveState() {
    if (runtimePointers()) {
        auto controlState = (GraphControlCurveState *) runtimePointers()->initialized;
        auto savedStorage = std::make_unique<GraphControlCurveStorage>();

        auto curveCount = *controlState->curveCount;
        savedStorage->curveCount = curveCount;
        memmove(savedStorage->curveStartVals, controlState->curveStartVals,
                sizeof(controlState->curveStartVals[0]) * curveCount);
        memmove(savedStorage->curveEndPositions, controlState->curveEndPositions,
                sizeof(controlState->curveEndPositions[0]) * curveCount);
        memmove(savedStorage->curveTension, controlState->curveTension,
                sizeof(controlState->curveTension[0]) * curveCount);
        memmove(savedStorage->curveStates, controlState->curveStates,
                sizeof(controlState->curveStates[0]) * curveCount);
        setSavedStorage(std::move(savedStorage));
    }
}

void GraphControl::restoreState() {
    if (_savedStorage && runtimePointers()) {
        auto controlState = (GraphControlCurveState *) runtimePointers()->initialized;

        auto curveCount = _savedStorage->curveCount;
        *controlState->curveCount = curveCount;
        memmove(controlState->curveStartVals, _savedStorage->curveStartVals,
                sizeof(_savedStorage->curveStartVals[0]) * curveCount);
        memmove(controlState->curveEndPositions, _savedStorage->curveEndPositions,
                sizeof(_savedStorage->curveEndPositions[0]) * curveCount);
        memmove(controlState->curveTension, _savedStorage->curveTension,
                sizeof(_savedStorage->curveTension[0]) * curveCount);
        memmove(controlState->curveStates, _savedStorage->curveStates,
                sizeof(_savedStorage->curveStates[0]) * curveCount);

        _savedStorage.reset();
    }
}

MaximCompiler::ControlInitializer GraphControl::getInitializer() {
    auto controlState = getCurveState();

    return MaximCompiler::ControlInitializer::graph(
        *controlState->curveCount, *controlState->curveCount + 1, controlState->curveStartVals,
        *controlState->curveCount, controlState->curveEndPositions, *controlState->curveCount,
        controlState->curveTension, *controlState->curveCount + 1, controlState->curveStates);
}

void GraphControl::setSavedStorage(std::unique_ptr<AxiomModel::GraphControlCurveStorage> storage) {
    _savedStorage = std::move(storage);
    _currentState.curveCount = &_savedStorage->curveCount;
    _currentState.curveStartVals = _savedStorage->curveStartVals;
    _currentState.curveEndPositions = _savedStorage->curveEndPositions;
    _currentState.curveTension = _savedStorage->curveTension;
    _currentState.curveStates = _savedStorage->curveStates;
}
