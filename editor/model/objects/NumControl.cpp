#include "NumControl.h"

#include "../ValueWriters.h"
#include "compiler/runtime/Control.h"
#include "compiler/runtime/ControlGroup.h"

using namespace AxiomModel;

NumControl::NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       bool showName, DisplayMode displayMode, Channel channel, MaximRuntime::NumValue value,
                       ModelRoot *root)
    : Control(ControlType::NUM_SCALAR, ConnectionWire::WireType::NUM, uuid, parentUuid, pos, size, selected,
              std::move(name), showName, root),
      _displayMode(displayMode), _channel(channel), _value(value) {
}

std::unique_ptr<NumControl> NumControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, bool showName,
                                               AxiomModel::NumControl::DisplayMode displayMode,
                                               AxiomModel::NumControl::Channel channel, MaximRuntime::NumValue value,
                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<NumControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, displayMode, channel, value, root);
}

std::unique_ptr<NumControl> NumControl::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    QPoint pos, QSize size, bool selected, QString name, bool showName,
                                                    AxiomModel::ModelRoot *root) {
    uint8_t displayModeInt;
    stream >> displayModeInt;
    uint8_t channelInt;
    stream >> channelInt;
    MaximRuntime::NumValue value;
    stream >> value;

    return create(uuid, parentUuid, pos, size, selected,
                  std::move(name), showName, (DisplayMode) displayModeInt, (Channel) channelInt, value, root);
}

void NumControl::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Control::serialize(stream, parent, withContext);
    stream << (uint8_t) _displayMode;
    stream << (uint8_t) _channel;
    stream << _value;
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

void NumControl::setValue(MaximRuntime::NumValue value) {
    setInternalValue(value);
    restoreValue();
}

void NumControl::doRuntimeUpdate() {
    saveValue();
}

void NumControl::saveValue() {
    if (!runtime() || !(*runtime())->group()) return;
    setInternalValue((*runtime())->group()->getNumValue());
}

void NumControl::restoreValue() {
    if (!runtime() || !(*runtime())->group()) return;
    (*runtime())->group()->setNumValue(_value);
}

void NumControl::setInternalValue(MaximRuntime::NumValue value) {
    value.left = value.left < 0 ? 0 : value.left > 1 ? 1 : value.left;
    value.right = value.right < 0 ? 0 : value.right > 1 ? 1 : value.right;

    if (value != _value) {
        _value = value;
        valueChanged.trigger(value);
    }
}
