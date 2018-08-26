#pragma once

#include "Control.h"

namespace AxiomModel {

    constexpr size_t GRAPH_CONTROL_CURVE_COUNT = 16;

    struct GraphControlState {
        uint8_t curveCount;
        float curveStartVals[GRAPH_CONTROL_CURVE_COUNT + 1];
        float curveEndPositions[GRAPH_CONTROL_CURVE_COUNT];
        float curveTension[GRAPH_CONTROL_CURVE_COUNT];
        int8_t curveStates[GRAPH_CONTROL_CURVE_COUNT];
    };

    class GraphControl : public Control {
    public:
        AxiomCommon::Event<float> zoomChanged;
        AxiomCommon::Event<float> scrollChanged;
        AxiomCommon::Event<> stateChanged;

        GraphControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                     bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, ModelRoot *root);

        static std::unique_ptr<GraphControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                    bool selected, QString name, bool showName,
                                                    const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                    ModelRoot *root);

        void doRuntimeUpdate() override;

        GraphControlState *state() const;

        float zoom() const { return _zoom; }

        void setZoom(float zoom);

        float scroll() const { return _scroll; }

        void setScroll(float scroll);

        void insertPoint(float time, float val);

        void removePoint(uint8_t index);

        void saveState() override;

        void restoreState() override;

    private:
        float _zoom = 0;
        float _scroll = 0;
        size_t _lastStateHash = 0;

        std::unique_ptr<GraphControlState> savedState;
    };
}
