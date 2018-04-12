#pragma once

#include "ConnectionSink.h"
#include "compiler/runtime/ValueOperator.h"

namespace AxiomModel {

    class MidiConnectionSink : public ConnectionSink {
    Q_OBJECT

    public:

        explicit MidiConnectionSink(NodeControl *control);

        MaximRuntime::MidiValue value() const { return m_value; }

    public slots:

        void setValue(MaximRuntime::MidiValue value);

    signals:

        void valueChanged(MaximRuntime::MidiValue newValue);

    private:

        MaximRuntime::MidiValue m_value;

    };

}
