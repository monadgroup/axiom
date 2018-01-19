#include "NodeValueControl.h"

using namespace AxiomModel;

NodeValueControl::NodeValueControl(Node *node, Type type, Channel channel, QPoint pos, QSize size)
        : NodeControl(node, channel, pos, size), type(type) {

}

void NodeValueControl::setValue(float value) {
    value = value < 0 ? 0 : value > 1 ? 1 : value;
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}
