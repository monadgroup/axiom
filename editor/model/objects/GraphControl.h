#pragma once

#include "Control.h"

namespace AxiomModel {

    constexpr size_t GRAPH_CONTROL_CURVE_COUNT = 16;

    struct GraphControlTimeState {
        uint32_t currentTimeSamples;
        uint8_t currentState;
    };

    struct GraphControlCurveState {
        uint8_t curveCount;
        double curveStartVals[GRAPH_CONTROL_CURVE_COUNT + 1];
        double curveEndPositions[GRAPH_CONTROL_CURVE_COUNT];
        double curveTension[GRAPH_CONTROL_CURVE_COUNT];
        uint8_t curveStates[GRAPH_CONTROL_CURVE_COUNT + 1];
    };

    class GraphControl : public Control {
    public:
        AxiomCommon::Event<float> zoomChanged;
        AxiomCommon::Event<float> scrollChanged;
        AxiomCommon::Event<> stateChanged;
        AxiomCommon::Event<> timeChanged;

        GraphControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                     bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                     std::unique_ptr<GraphControlCurveState> savedState, ModelRoot *root);

        static std::unique_ptr<GraphControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                    bool selected, QString name, bool showName,
                                                    const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                    std::unique_ptr<GraphControlCurveState> savedState,
                                                    ModelRoot *root);

        QString debugName() override;

        void doRuntimeUpdate() override;

        GraphControlTimeState *getTimeState() const;

        GraphControlCurveState *getCurveState() const;

        float zoom() const { return _zoom; }

        void setZoom(float zoom);

        float scroll() const { return _scroll; }

        void setScroll(float scroll);

        std::optional<uint8_t> determineInsertIndex(double time);

        void insertPoint(uint8_t index, double time, double val, double tension, uint8_t curveState);

        void movePoint(uint8_t index, double time, double value);

        void setPointTag(uint8_t index, uint8_t tag);

        void setCurveTension(uint8_t index, double tension);

        void removePoint(uint8_t index);

        void saveState() override;

        void restoreState() override;

        MaximCompiler::ControlInitializer getInitializer() override;

    private:
        float _zoom = 0;
        float _scroll = 0;
        size_t _lastStateHash = 0;
        uint32_t _lastTime = 0;

        std::unique_ptr<GraphControlCurveState> _savedState;
    };
}
