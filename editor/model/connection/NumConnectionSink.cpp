#include "NumConnectionSink.h"

using namespace AxiomModel;

NumConnectionSink::NumConnectionSink() : ConnectionSink(Type::NUMBER) {}

void NumConnectionSink::setValue(NumValue value) {
    value.left = value.left < 0 ? 0 : value.left > 1 ? 1 : value.left;
    value.right = value.right < 0 ? 0 : value.right > 1 ? 1 : value.right;

    if (value != m_value) {
        auto oldVal = m_value;
        m_value = value;
        emit valueChanged(value);

        if (value.left != oldVal.left) {
            emit leftChanged(value.left);
        }
        if (value.right != oldVal.right) {
            emit rightChanged(value.right);
        }
    }
}

void NumConnectionSink::setLeft(float leftValue) {
    setValue({ leftValue, m_value.right });
}

void NumConnectionSink::setRight(float rightValue) {
    setValue({ m_value.left, rightValue });
}
