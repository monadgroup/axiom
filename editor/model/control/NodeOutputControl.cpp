#include "NodeOutputControl.h"

#include "compiler/runtime/OutputControl.h"
#include "../node/OutputNode.h"

using namespace AxiomModel;

NodeOutputControl::NodeOutputControl(OutputNode *node, MaximRuntime::OutputControl *runtime)
    : NodeControl(node, runtime, QPoint(0, 0), QSize(2, 2)), m_sink(runtime) {
    initSink();
}
