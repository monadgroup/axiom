#include "NodeExtractControl.h"

using namespace AxiomModel;

NodeExtractControl::NodeExtractControl(Node *node, MaximRuntime::Control *runtime, ConnectionSink::Type baseType,
                                       QPoint pos, QSize size)
    : NodeControl(node, runtime, pos, size), m_sink(runtime, baseType) {
    initSink();

    connect(&m_sink, &ExtractConnectionSink::activeSlotsChanged,
            this, &NodeExtractControl::activeSlotsChanged);
}

void NodeExtractControl::doRuntimeUpdate() {
    // todo
}
