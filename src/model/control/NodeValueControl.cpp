#include "NodeValueControl.h"

using namespace AxiomModel;

NodeValueControl::NodeValueControl(NodeSurface *parent, Type type, Channel channel)
        : NodeControl(parent, channel), type(type) {

}

void NodeValueControl::setValue(float value) {
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}
