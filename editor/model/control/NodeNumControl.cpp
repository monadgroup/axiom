#include "NodeNumControl.h"

#include <cassert>

#include "../node/NodeSurface.h"

using namespace AxiomModel;

NodeNumControl::NodeNumControl(Node *node, MaximRuntime::Control *runtime, QPoint pos, QSize size)
        : NodeControl(node, runtime, pos, size), m_sink(runtime) {
    initSink();

    connect(&m_sink, &NumConnectionSink::valueChanged,
            this, &NodeNumControl::valueChanged);
}

std::unique_ptr<GridItem> NodeNumControl::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    assert(false);
    throw;
}

void NodeNumControl::setValue(NumValue value) {
    m_sink.setValue(value);
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
