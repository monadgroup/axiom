#include "GraphControl.h"

using namespace AxiomModel;

GraphControl::GraphControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                           QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                           AxiomModel::ModelRoot *root)
    : Control(ControlType::GRAPH, ConnectionWire::WireType::NUM, uuid, parentUuid, pos, size, selected, std::move(name),
              showName, exposerUuid, exposingUuid, root) {}

std::unique_ptr<GraphControl> GraphControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                   const QUuid &exposingUuid, AxiomModel::ModelRoot *root) {
    return std::make_unique<GraphControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                          exposingUuid, root);
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
