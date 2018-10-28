#include "NumControl.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

NumControl::NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, DisplayMode displayMode,
                       float minValue, float maxValue, uint32_t step, NumValue value, ModelRoot *root)
    : Control(ControlType::NUM_SCALAR, ConnectionWire::WireType::NUM, QSize(1, 1), uuid, parentUuid, pos, size,
              selected, std::move(name), showName, exposerUuid, exposingUuid, root),
      _displayMode(displayMode), _minValue(minValue), _maxValue(maxValue), _step(step), _value(value) {
    if (!exposingUuid.isNull()) {
        findLater(AxiomCommon::dynamicCastWatch<NumControl *>(root->controls()), exposingUuid)
            ->then([this](NumControl *exposing) {
                exposing->displayModeChanged.connect(this, &NumControl::setDisplayMode);
                exposing->rangeChanged.connect(this, &NumControl::setRange);
            });
    }
}

std::unique_ptr<NumControl> NumControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                               const QUuid &exposingUuid,
                                               AxiomModel::NumControl::DisplayMode displayMode, float minValue,
                                               float maxValue, uint32_t step, NumValue value,
                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<NumControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                        exposingUuid, displayMode, minValue, maxValue, step, value, root);
}

QString NumControl::debugName() {
    return "NumControl '" + name() + "'";
}

void NumControl::setDisplayMode(AxiomModel::NumControl::DisplayMode displayMode) {
    // if we're exposing, set it on the underlying control
    if (!exposingUuid().isNull()) {
        auto exposingControl = dynamic_cast<NumControl *>(find(root()->controls().sequence(), exposingUuid()));
        assert(exposingControl);
        exposingControl->setDisplayMode(displayMode);
    }

    if (displayMode != _displayMode) {
        _displayMode = displayMode;
        displayModeChanged(displayMode);
    }
}

void NumControl::setRange(float minValue, float maxValue, uint32_t step) {
    // if we're exposing, set it on the underlying control
    if (!exposingUuid().isNull()) {
        auto exposingControl = dynamic_cast<NumControl *>(find(root()->controls().sequence(), exposingUuid()));
        assert(exposingControl);
        exposingControl->setRange(minValue, maxValue, step);
    }

    if (minValue != _minValue || maxValue != _maxValue || step != _step) {
        _minValue = minValue;
        _maxValue = maxValue;
        _step = step;
        rangeChanged(minValue, maxValue, step);
    }
}

void NumControl::setValue(NumValue value) {
    setInternalValue(value);
    if (runtimePointers()) *(NumValue *) runtimePointers()->value = value;
}

void NumControl::doRuntimeUpdate() {
    if (runtimePointers()) setInternalValue(*(NumValue *) runtimePointers()->value);
}

void NumControl::setInternalValue(NumValue value) {
    if (value != _value) {
        _value = value;
        valueChanged(value);
    }
}
