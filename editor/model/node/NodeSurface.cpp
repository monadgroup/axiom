#include "NodeSurface.h"

#include "Node.h"

using namespace AxiomModel;

NodeSurface::NodeSurface(Node *node) : GridSurface(QPoint(0, 0), QPoint(node->size().width(), node->size().height())),
                                       node(node) {
    connect(node, &Node::sizeChanged,
            this, &NodeSurface::setSize);
    connect(node, &Node::removed,
            [this]() {
                while (!items().empty()) {
                    items()[0]->remove();
                }
            });
}

void NodeSurface::setSize(QSize size) {
    grid.maxRect = schematicToNodeSurface(QPoint(size.width(), size.height()));
}
