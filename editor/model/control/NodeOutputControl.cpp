#include "NodeOutputControl.h"

#include "../node/Node.h"

using namespace AxiomModel;

NodeOutputControl::NodeOutputControl(Node *node, Channel channel)
        : NodeControl(node, channel), source(node->parentSurface) {
    connect(this, &NodeOutputControl::worldPosChanged,
            &source, &ConnectionSource::setPos);
}
