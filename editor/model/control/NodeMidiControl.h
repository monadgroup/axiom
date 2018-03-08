#pragma once

#include "NodeControl.h"
#include "../connection/MidiConnectionSink.h"

namespace AxiomModel {

    class Node;

    class NodeMidiControl : public NodeControl {
        Q_OBJECT

    public:
        enum class Mode {
            PLUG,
            PIANO
        };

        NodeMidiControl(Node *node, MaximRuntime::Control *runtime, QPoint pos, QSize size);

        MidiConnectionSink *sink() override { return &m_sink; }

        MaximRuntime::MidiValue value() const { return m_sink.value(); }

        Mode mode() const { return m_mode; }

        bool isResizable() const override { return true; }

    public slots:

        void doRuntimeUpdate() override;

        void setValue(MaximRuntime::MidiValue value, bool setRuntime = true);

        void pushEvent(MaximRuntime::MidiEventValue event);

        void setMode(Mode mode);

    signals:

        void valueChanged(MaximRuntime::MidiValue newValue);

        void modeChanged(Mode newMode);

    private:

        Mode m_mode = Mode::PLUG;
        MidiConnectionSink m_sink;

    };

}
