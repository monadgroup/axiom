#include "NodeNumControl.h"

#include <cassert>

#include "../node/NodeSurface.h"

using namespace AxiomModel;

NodeNumControl::NodeNumControl(Node *node, QString name, QPoint pos, QSize size)
        : NodeControl(node, std::move(name), pos, size) {
    initSink();

    connect(m_sink.get(), &NumConnectionSink::valueChanged,
            this, &NodeNumControl::valueChanged);
}

std::unique_ptr<GridItem> NodeNumControl::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    auto nodeSurface = dynamic_cast<NodeSurface *>(newParent);
    assert(nodeSurface != nullptr);

    auto control = std::make_unique<NodeNumControl>(nodeSurface->node, name(), pos(), size());
    control->setValue(value());
    return std::move(control);
}

void NodeNumControl::setValue(NumValue value) {
    m_sink->setValue(value);
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
