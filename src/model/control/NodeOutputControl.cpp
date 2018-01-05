#include "NodeOutputControl.h"

using namespace AxiomModel;

NodeOutputControl::NodeOutputControl(Node *node, Channel channel) : NodeControl(node, channel) {
    connect(this, &NodeOutputControl::worldPosChanged,
            &source, &ConnectionSource::setPos);
}
