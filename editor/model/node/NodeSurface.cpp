#include "NodeSurface.h"

#include "Node.h"

using namespace AxiomModel;

NodeSurface::NodeSurface(Node *node) : GridSurface(QPoint(0, 0), QPoint(node->size().width(), node->size().height())),
                                       node(node) {
    connect(node, &Node::sizeChanged,
            this, &NodeSurface::setSize);
}

void NodeSurface::setLocked(bool newLocked) {
    if (newLocked != m_locked) {
        if (newLocked) deselectAll();

        m_locked = newLocked;
        emit lockedChanged(newLocked);
    }
}

void NodeSurface::setSize(QSize size) {
    grid.maxRect = schematicToNodeSurface(QPoint(size.width(), size.height()));
}
