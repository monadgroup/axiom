#include "NumConnectionSink.h"

using namespace AxiomModel;

NumConnectionSink::NumConnectionSink(NodeControl *control) : ConnectionSink(Type::NUMBER, control) {}

void NumConnectionSink::setValue(MaximRuntime::NumValue value) {
    value.left = value.left < 0 ? 0 : value.left > 1 ? 1 : value.left;
    value.right = value.right < 0 ? 0 : value.right > 1 ? 1 : value.right;

    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}
