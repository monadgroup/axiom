#pragma once

#include "ConnectionSink.h"
#include "compiler/common/FormType.h"
#include "compiler/runtime/ValueOperator.h"

namespace AxiomModel {

    class NumConnectionSink : public ConnectionSink {
    Q_OBJECT

    public:

        explicit NumConnectionSink(NodeControl *control);

        MaximRuntime::NumValue value() const { return m_value; }

    public slots:

        void setValue(MaximRuntime::NumValue value);

    signals:

        void valueChanged(MaximRuntime::NumValue newValue);

    private:

        MaximRuntime::NumValue m_value;
    };

}
