#pragma once

#include "NodeControl.h"
#include "../connection/NumConnectionSink.h"

namespace AxiomModel {

    class Node;

    class NodeNumControl : public NodeControl {
    Q_OBJECT

    public:
        enum class Mode {
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

        NodeNumControl(Node *node, QString name, QPoint pos, QSize size);

        MaximCommon::ControlType type() const { return MaximCommon::ControlType::NUMBER; }

        NumConnectionSink *sink() override { return &m_sink; }

        MaximRuntime::NumValue value() const { return m_sink.value(); }

        Mode mode() const { return m_mode; }

        Channel channel() const { return m_channel; }

        bool isResizable() const override { return true; }

    public slots:

        void doRuntimeUpdate() override;

        void startSetValue();

        void setValue(MaximRuntime::NumValue value, bool setRuntime = true);

        void endSetValue();

        void saveValue() override;

        void restoreValue() override;

        void setMode(Mode mode);

        void setModeNoOp(Mode mode);

        void setChannel(Channel channel);

        void serialize(QDataStream &stream, QPoint offset) const override;

        void deserialize(QDataStream &stream, QPoint offset) override;

    signals:

        void valueChanged(MaximRuntime::NumValue newValue);

        void modeChanged(Mode newMode);

        void channelChanged(Channel newChannel);

    private:

        Mode m_mode = Mode::KNOB;
        Channel m_channel = Channel::BOTH;
        NumConnectionSink m_sink;

        MaximRuntime::NumValue beforeVal;
    };

}
