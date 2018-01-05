#include "NodeValueControl.h"

using namespace AxiomModel;

NodeValueControl::NodeValueControl(Node *node, Type type, Channel channel)
        : NodeControl(node, channel), type(type) {

}

void NodeValueControl::setValue(float value) {
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}
