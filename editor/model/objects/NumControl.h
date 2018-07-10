#pragma once

#include "../Value.h"
#include "Control.h"
#include "common/Event.h"

namespace AxiomModel {

    class NumControl : public Control {
    public:
        enum class DisplayMode { PLUG, KNOB, SLIDER_H, SLIDER_V, TOGGLE };

        enum class Channel { LEFT = 1 << 0, RIGHT = 1 << 1, BOTH = LEFT | RIGHT };

        AxiomCommon::Event<DisplayMode> displayModeChanged;
        AxiomCommon::Event<Channel> channelChanged;
        AxiomCommon::Event<const NumValue &> valueChanged;

        NumControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                   bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, DisplayMode displayMode,
                   Channel channel, NumValue value, ModelRoot *root);

        static std::unique_ptr<NumControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                  bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                  const QUuid &exposingUuid, DisplayMode displayMode, Channel channel,
                                                  NumValue value, ModelRoot *root);

        static std::unique_ptr<NumControl> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                       QPoint pos, QSize size, bool selected, QString name,
                                                       bool showName, const QUuid &exposerUuid,
                                                       const QUuid &exposingUuid, ReferenceMapper *ref,
                                                       ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        DisplayMode displayMode() const { return _displayMode; }

        void setDisplayMode(DisplayMode displayMode);

        Channel channel() const { return _channel; }

        void setChannel(Channel channel);

        const NumValue &value() const { return _value; }

        void doRuntimeUpdate() override;

        void setValue(NumValue value);

        void saveValue() override;

        void restoreValue() override;

    private:
        DisplayMode _displayMode;
        Channel _channel;
        NumValue _value;

        void setInternalValue(NumValue value);
    };
}
