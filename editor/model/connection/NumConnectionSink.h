#pragma once

#include "ConnectionSink.h"

namespace AxiomModel {

    struct NumValue {
        float left = 0;
        float right = 0;

        bool operator==(const NumValue &other) const {
            return left == other.left && right == other.right;
        }
        bool operator!=(const NumValue &other) const {
            return !(*this == other);
        }
    };

    class NumConnectionSink : public ConnectionSink {
        Q_OBJECT

    public:

        NumConnectionSink();

        NumValue value() const { return m_value; }

    public slots:

        void setValue(NumValue value);

        void setLeft(float leftValue);

        void setRight(float rightValue);

    signals:

        void valueChanged(NumValue newValue);

        void leftChanged(float newLeft);

        void rightChanged(float newRight);

    private:

        NumValue m_value;
    };

}
