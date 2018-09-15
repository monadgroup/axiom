#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetNumRangeAction : public Action {
    public:
        SetNumRangeAction(const QUuid &uuid, float beforeMin, float beforeMax, float afterMin, float afterMax,
                          ModelRoot *root);

        static std::unique_ptr<SetNumRangeAction> create(const QUuid &uuid, float beforeMin, float beforeMax,
                                                         float afterMin, float afterMax, ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &uuid() const { return _uuid; }

        float beforeMin() const { return _beforeMin; }

        float beforeMax() const { return _beforeMax; }

        float afterMin() const { return _afterMin; }

        float afterMax() const { return _afterMax; }

    private:
        QUuid _uuid;
        float _beforeMin;
        float _beforeMax;
        float _afterMin;
        float _afterMax;
    };
}
