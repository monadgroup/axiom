#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetNumRangeAction : public Action {
    public:
        SetNumRangeAction(const QUuid &uuid, float beforeMin, float beforeMax, uint32_t beforeStep, float afterMin,
                          float afterMax, uint32_t afterStep, ModelRoot *root);

        static std::unique_ptr<SetNumRangeAction> create(const QUuid &uuid, float beforeMin, float beforeMax,
                                                         uint32_t beforeStep, float afterMin, float afterMax,
                                                         uint32_t afterStep, ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &uuid() const { return _uuid; }

        float beforeMin() const { return _beforeMin; }

        float beforeMax() const { return _beforeMax; }

        uint32_t beforeStep() const { return _beforeStep; }

        float afterMin() const { return _afterMin; }

        float afterMax() const { return _afterMax; }

        uint32_t afterStep() const { return _afterStep; }

    private:
        QUuid _uuid;
        float _beforeMin;
        float _beforeMax;
        uint32_t _beforeStep;
        float _afterMin;
        float _afterMax;
        uint32_t _afterStep;
    };
}
