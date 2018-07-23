#pragma once

#include "Control.h"

namespace AxiomModel {

    struct GraphControlState {
        uint8_t curveCount;
        float curveStartVals[9];
        float curveEndPositions[8];
        float curveTension[8];
        int8_t curveStates[8];
    };

    class GraphControl : public Control {
    public:
        AxiomCommon::Event<float> zoomChanged;

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

    private:
        float _zoom = 0;
    };
}
