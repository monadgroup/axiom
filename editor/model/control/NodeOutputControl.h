#pragma once

#include "NodeControl.h"
#include "../connection/NumConnectionSink.h"

namespace MaximRuntime {
    class OutputControl;
}

namespace AxiomModel {

    class OutputNode;

    class NodeOutputControl : public NodeControl {
    Q_OBJECT

    public:

        NodeOutputControl(OutputNode *node, MaximRuntime::OutputControl *runtime);

        NumConnectionSink *sink() override { return &m_sink; }

        bool isMovable() const override { return false; }

        bool isResizable() const override { return false; }

        void doRuntimeUpdate() override {}

    private:

        NumConnectionSink m_sink;

    };

}
