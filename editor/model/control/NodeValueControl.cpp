#include "NodeValueControl.h"

#include <cassert>

#include "../node/NodeSurface.h"

using namespace AxiomModel;

NodeValueControl::NodeValueControl(Node *node, QString name, Type type, Channel channel, QPoint pos, QSize size)
        : NodeControl(node, std::move(name), pos, size), type(type), channel(channel) {
    initSink();

    switch (channel) {
        case Channel::BOTH:
        case Channel::LEFT:
            connect(m_sink.get(), &NumConnectionSink::leftChanged,
                    this, &NodeValueControl::valueChanged);
            break;
        case Channel::RIGHT:
            connect(m_sink.get(), &NumConnectionSink::rightChanged,
                    this, &NodeValueControl::valueChanged);
            break;
    }
}

std::unique_ptr<GridItem> NodeValueControl::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    auto nodeSurface = dynamic_cast<NodeSurface *>(newParent);
    assert(nodeSurface != nullptr);

    auto control = std::make_unique<NodeValueControl>(nodeSurface->node, name(), type, channel, pos(), size());
    control->setValue(value());
    return std::move(control);
}

void NodeValueControl::setValue(float value) {
    switch (channel) {
        case Channel::LEFT:
            m_sink->setLeft(value);
            break;
        case Channel::RIGHT:
            m_sink->setRight(value);
            break;
        case Channel::BOTH:
            m_sink->setValue({value, value});
            break;
    }
}
