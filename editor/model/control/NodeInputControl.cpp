#include "NodeInputControl.h"

using namespace AxiomModel;

NodeInputControl::NodeInputControl(Node *node, Channel channel) : NodeControl(node, channel) {
    connect(this, &NodeInputControl::worldPosChanged,
            &sink, &ConnectionSink::setPos);
}
