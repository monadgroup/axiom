#include "NumControl.h"

using namespace AxiomModel;

NumControl::NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, DisplayMode displayMode,
                       Channel channel, NumValue value, ModelRoot *root)
    : Control(ControlType::NUM_SCALAR, ConnectionWire::WireType::NUM, uuid, parentUuid, pos, size, selected,
              std::move(name), showName, exposerUuid, exposingUuid, root),
      _displayMode(displayMode), _channel(channel), _value(value) {}

std::unique_ptr<NumControl> NumControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                               const QUuid &exposingUuid,
                                               AxiomModel::NumControl::DisplayMode displayMode,
                                               AxiomModel::NumControl::Channel channel, NumValue value,
                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<NumControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                        exposingUuid, displayMode, channel, value, root);
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
