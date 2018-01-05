#pragma once

#include "NodeControl.h"
#include "../connection/ConnectionSource.h"

namespace AxiomModel {

    class NodeOutputControl : public NodeControl {
        Q_OBJECT

    public:
        ConnectionSource source;

        NodeOutputControl(Node *node, Channel channel);
    };

}
