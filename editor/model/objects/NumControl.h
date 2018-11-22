#pragma once

#include "../Value.h"
#include "Control.h"
#include "common/Event.h"

namespace AxiomModel {

    class NumControl : public Control {
    public:
        enum class DisplayMode { PLUG, KNOB, SLIDER_H, SLIDER_V, TOGGLE };

        AxiomCommon::Event<DisplayMode> displayModeChanged;
        AxiomCommon::Event<float, float, uint32_t> rangeChanged;
        AxiomCommon::Event<const NumValue &> valueChanged;

        NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                   bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, DisplayMode displayMode,
                   double minValue, double maxValue, uint32_t step, NumValue value, ModelRoot *root);

        static std::unique_ptr<NumControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                  bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                  const QUuid &exposingUuid, DisplayMode displayMode, double minValue,
                                                  double maxValue, uint32_t step, NumValue value, ModelRoot *root);

        QString debugName() override;

        DisplayMode displayMode() const { return _displayMode; }

        void setDisplayMode(DisplayMode displayMode);

        double minValue() const { return _minValue; }

        double maxValue() const { return _maxValue; }

        uint32_t step() const { return _step; }

        void setRange(double minValue, double maxValue, uint32_t step);

        const NumValue &value() const { return _value; }

        void doRuntimeUpdate() override;

        void saveState() override;

        void restoreState() override;

        void setValue(NumValue value);

    private:
        DisplayMode _displayMode;
        double _minValue;
        double _maxValue;
        uint32_t _step;
        NumValue _value;

        void setInternalValue(NumValue value);
    };
}
