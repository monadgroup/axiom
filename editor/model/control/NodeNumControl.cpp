#include <iostream>
#include <iomanip>
#include "NodeNumControl.h"

#include "../node/NodeSurface.h"
#include "compiler/runtime/Control.h"
#include "compiler/runtime/ControlGroup.h"
#include "compiler/runtime/Node.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

NodeNumControl::NodeNumControl(Node *node, MaximRuntime::Control *runtime, QPoint pos, QSize size)
        : NodeControl(node, runtime, pos, size), m_sink(runtime) {
    initSink();

    connect(&m_sink, &NumConnectionSink::valueChanged,
            this, &NodeNumControl::valueChanged);
}

void NodeNumControl::doRuntimeUpdate() {
    setValue(runtime()->group()->getNumValue(), false);
}

void NodeNumControl::setValue(MaximRuntime::NumValue value, bool setRuntime) {
    m_sink.setValue(value);
    if (setRuntime) runtime()->group()->setNumValue(value);
}

void NodeNumControl::setMode(Mode mode) {
    if (mode != m_mode) {
        m_mode = mode;
        emit modeChanged(mode);
    }
}

void NodeNumControl::setChannel(Channel channel) {
    if (channel != m_channel) {
        m_channel = channel;
        emit channelChanged(channel);
    }
}
