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
                   float minValue, float maxValue, uint32_t step, NumValue value, ModelRoot *root);

        static std::unique_ptr<NumControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                  bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                  const QUuid &exposingUuid, DisplayMode displayMode, float minValue,
                                                  float maxValue, uint32_t step, NumValue value, ModelRoot *root);

        DisplayMode displayMode() const { return _displayMode; }

        void setDisplayMode(DisplayMode displayMode);

        float minValue() const { return _minValue; }

        float maxValue() const { return _maxValue; }

        uint32_t step() const { return _step; }

        void setRange(float minValue, float maxValue, uint32_t step);

        const NumValue &value() const { return _value; }

        void doRuntimeUpdate() override;

        void setValue(NumValue value);

    private:
        DisplayMode _displayMode;
        float _minValue;
        float _maxValue;
        uint32_t _step;
        NumValue _value;

        void setInternalValue(NumValue value);
    };
}
