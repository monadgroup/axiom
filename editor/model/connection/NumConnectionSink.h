#pragma once

#include "ConnectionSink.h"
#include "compiler/common/FormType.h"

namespace AxiomModel {

    struct NumValue {
        float left = 0;
        float right = 0;
        MaximCommon::FormType form = MaximCommon::FormType::LINEAR;
        bool active = false;

        bool operator==(const NumValue &other) const {
            return left == other.left && right == other.right && form == other.form && active == other.active;
        }

        bool operator!=(const NumValue &other) const {
            return !(*this == other);
        }

        NumValue withLR(float l, float r) const {
            return { l, r, form, active };
        }

        NumValue withL(float l) const {
            return { l, right, form, active };
        }

        NumValue withR(float r) const {
            return { left, r, form, active };
        }
    };

    class NumConnectionSink : public ConnectionSink {
    Q_OBJECT

    public:

        NumConnectionSink();

        NumValue value() const { return m_value; }

    public slots:

        void setValue(NumValue value);

    signals:

        void valueChanged(NumValue newValue);

    private:

        NumValue m_value;
    };

}
