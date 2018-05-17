#include "NumControl.h"

#include "../ValueWriters.h"

using namespace AxiomModel;

NumControl::NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       DisplayMode displayMode, Channel channel, MaximRuntime::NumValue value, ModelRoot *root)
    : Control(ControlType::NUM_SCALAR, ConnectionWire::WireType::NUM, uuid, parentUuid, pos, size, selected,
              std::move(name), root),
      _displayMode(displayMode), _channel(channel), _value(value) {
}

std::unique_ptr<NumControl> NumControl::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    QPoint pos, QSize size, bool selected, QString name,
                                                    AxiomModel::ModelRoot *root) {
    uint8_t displayModeInt;
    stream >> displayModeInt;
    uint8_t channelInt;
    stream >> channelInt;
    MaximRuntime::NumValue value;
    stream >> value;

    return std::make_unique<NumControl>(uuid, parentUuid, pos, size, selected, name, (DisplayMode) displayModeInt,
                                        (Channel) channelInt, value, root);
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

void NumControl::setValue(const MaximRuntime::NumValue &value) {
    if (value != _value) {
        _value = value;
        valueChanged.trigger(value);
    }
}
