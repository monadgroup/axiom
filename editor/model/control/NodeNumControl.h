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

        NodeNumControl(Node *node, MaximRuntime::Control *runtime, QPoint pos, QSize size);

        NumConnectionSink *sink() override { return &m_sink; }

        NumValue value() const { return m_sink.value(); }

        Mode mode() const { return m_mode; }

        Channel channel() const { return m_channel; }

        bool isResizable() const override { return true; }

        void doRuntimeUpdate() override;

    public slots:

        void setValue(NumValue value);

        void setMode(Mode mode);

        void setChannel(Channel channel);

    signals:

        void valueChanged(NumValue newValue);

        void modeChanged(Mode newMode);

        void channelChanged(Channel newChannel);

    private:

        Mode m_mode = Mode::KNOB;
        Channel m_channel = Channel::BOTH;
        NumConnectionSink m_sink;
    };

}
