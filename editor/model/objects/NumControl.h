#pragma once

#include "common/Event.h"
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

        AxiomCommon::Event<DisplayMode> displayModeChanged;
        AxiomCommon::Event<Channel> channelChanged;
        AxiomCommon::Event<const MaximRuntime::NumValue &> valueChanged;

        NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                   bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, DisplayMode displayMode,
                   Channel channel, MaximRuntime::NumValue value, ModelRoot *root);

        static std::unique_ptr<NumControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                  bool selected, QString name, bool showName,
                                                  const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                  DisplayMode displayMode, Channel channel,
                                                  MaximRuntime::NumValue value, ModelRoot *root);

        static std::unique_ptr<NumControl>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                    bool selected, QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                    ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        DisplayMode displayMode() const { return _displayMode; }

        void setDisplayMode(DisplayMode displayMode);

        Channel channel() const { return _channel; }

        void setChannel(Channel channel);

        const MaximRuntime::NumValue &value() const { return _value; }

        void setValue(MaximRuntime::NumValue value);

        void doRuntimeUpdate() override;

        void saveValue() override;

        void restoreValue() override;

    private:
        DisplayMode _displayMode;
        Channel _channel;
        MaximRuntime::NumValue _value;

        void setInternalValue(MaximRuntime::NumValue value);
    };

}
