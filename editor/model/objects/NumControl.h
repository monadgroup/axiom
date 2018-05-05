#pragma once

#include "Control.h"
#include "compiler/runtime/ValueOperator.h"

namespace AxiomModel {

    class NumControl : public Control {
    public:
        enum class DisplayMode {
            PLUG,
            KNOB,
            SLIDER_H,
            SLIDER_V,
            TOGGLE
        };

        enum class Channel {
            LEFT = 1 << 0,
            RIGHT = 1 << 1,
            BOTH = LEFT | RIGHT
        };

        Event<DisplayMode> displayModeChanged;
        Event<Channel> channelChanged;
        Event<const MaximRuntime::NumValue &> valueChanged;

        NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, DisplayMode displayMode, Channel channel, MaximRuntime::NumValue value, ModelRoot *root);

        static std::unique_ptr<NumControl> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        DisplayMode displayMode() const { return _displayMode; }

        void setDisplayMode(DisplayMode displayMode);

        Channel channel() const { return _channel; }

        void setChannel(Channel channel);

        const MaximRuntime::NumValue &value() const { return _value; }

        void setValue(const MaximRuntime::NumValue &value);

    private:
        DisplayMode _displayMode;
        Channel _channel;
        MaximRuntime::NumValue _value;
    };

}
