#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetNumRangeAction : public Action {
    public:
        SetNumRangeAction(const QUuid &uuid, double beforeMin, double beforeMax, uint32_t beforeStep, double afterMin,
                          double afterMax, uint32_t afterStep, ModelRoot *root);

        static std::unique_ptr<SetNumRangeAction> create(const QUuid &uuid, double beforeMin, double beforeMax,
                                                         uint32_t beforeStep, double afterMin, double afterMax,
                                                         uint32_t afterStep, ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        double beforeMin() const { return _beforeMin; }

        double beforeMax() const { return _beforeMax; }

        uint32_t beforeStep() const { return _beforeStep; }

        double afterMin() const { return _afterMin; }

        double afterMax() const { return _afterMax; }

        uint32_t afterStep() const { return _afterStep; }

    private:
        QUuid _uuid;
        double _beforeMin;
        double _beforeMax;
        uint32_t _beforeStep;
        double _afterMin;
        double _afterMax;
        uint32_t _afterStep;
    };
}
