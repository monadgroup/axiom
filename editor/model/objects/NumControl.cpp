#include "NumControl.h"

using namespace AxiomModel;

NumControl::NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, DisplayMode displayMode,
                       float minValue, float maxValue, NumValue value, ModelRoot *root)
    : Control(ControlType::NUM_SCALAR, ConnectionWire::WireType::NUM, QSize(1, 1), uuid, parentUuid, pos, size,
              selected, std::move(name), showName, exposerUuid, exposingUuid, root),
      _displayMode(displayMode), _minValue(minValue), _maxValue(maxValue), _value(value) {}

std::unique_ptr<NumControl> NumControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                               const QUuid &exposingUuid,
                                               AxiomModel::NumControl::DisplayMode displayMode, float minValue,
                                               float maxValue, NumValue value, AxiomModel::ModelRoot *root) {
    return std::make_unique<NumControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                        exposingUuid, displayMode, minValue, maxValue, value, root);
}

void NumControl::setDisplayMode(AxiomModel::NumControl::DisplayMode displayMode) {
    if (displayMode != _displayMode) {
        _displayMode = displayMode;
        displayModeChanged.trigger(displayMode);
    }
}

void NumControl::setChannel(AxiomModel::NumControl::Channel channel) {
    if (channel != _channel) {
        _channel = channel;
        channelChanged.trigger(channel);
    }
}

void NumControl::setMinValue(float minValue) {
    if (minValue != _minValue) {
        _minValue = minValue;
        minValueChanged.trigger(minValue);
    }
}

void NumControl::setMaxValue(float maxValue) {
    if (maxValue != _maxValue) {
        _maxValue = maxValue;
        maxValueChanged.trigger(maxValue);
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
        valueChanged.trigger(value);
    }
}
