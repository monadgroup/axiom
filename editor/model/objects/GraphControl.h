#pragma once

#include "Control.h"

namespace AxiomModel {

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

        float zoom() const { return _zoom; }

        void setZoom(float zoom);

    private:
        float _zoom = 0;
    };
}
