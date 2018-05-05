#include "NumControl.h"

#include <utility>

using namespace AxiomModel;

NumControl::NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       DisplayMode displayMode, Channel channel, ModelRoot *root)
    : Control(ControlType::NUM_SCALAR, ValueType::NUM, uuid, parentUuid, pos, size, selected, std::move(name), root),
      _displayMode(displayMode), _channel(channel) {
}

std::unique_ptr<NumControl> NumControl::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    QPoint pos, QSize size, bool selected, QString name,
                                                    AxiomModel::ModelRoot *root) {
    uint8_t displayModeInt; stream >> displayModeInt;
    uint8_t channelInt; stream >> channelInt;

    return std::make_unique<NumControl>(uuid, parentUuid, pos, size, selected, name, (DisplayMode) displayModeInt, (Channel) channelInt, root);
}

void NumControl::serialize(QDataStream &stream) const {
    Control::serialize(stream);
    stream << (uint8_t) _displayMode;
    stream << (uint8_t) _channel;
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
