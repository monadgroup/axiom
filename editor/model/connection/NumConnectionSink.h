#pragma once

#include "ConnectionSink.h"
#include "compiler/common/FormType.h"
#include "compiler/runtime/ValueReader.h"

namespace AxiomModel {

    class NumConnectionSink : public ConnectionSink {
    Q_OBJECT

    public:

        NumConnectionSink(MaximRuntime::Control *runtime);

        MaximRuntime::NumValue value() const { return m_value; }

    public slots:

        void setValue(MaximRuntime::NumValue value);

    signals:

        void valueChanged(MaximRuntime::NumValue newValue);

    private:

        MaximRuntime::NumValue m_value;
    };

}
