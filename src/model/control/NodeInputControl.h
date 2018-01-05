#pragma once

#include "NodeControl.h"
#include "../connection/ConnectionSink.h"

namespace AxiomModel {

    class NodeInputControl : public NodeControl {
        Q_OBJECT

    public:
        ConnectionSink sink;

        NodeInputControl(Node *node, Channel channel);
    };

}
