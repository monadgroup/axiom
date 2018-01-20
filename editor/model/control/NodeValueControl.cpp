#include "NodeValueControl.h"

#include <cassert>

#include "../node/NodeSurface.h"

using namespace AxiomModel;

NodeValueControl::NodeValueControl(Node *node, Type type, Channel channel, QPoint pos, QSize size)
        : NodeControl(node, channel, pos, size), type(type) {

}

std::unique_ptr<GridItem> NodeValueControl::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    auto nodeSurface = dynamic_cast<NodeSurface *>(newParent);
    assert(nodeSurface != nullptr);

    auto control = std::make_unique<NodeValueControl>(nodeSurface->node, type, channel, pos(), size());
    control->setValue(value());
    return std::move(control);
}

void NodeValueControl::setValue(float value) {
    value = value < 0 ? 0 : value > 1 ? 1 : value;
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}
