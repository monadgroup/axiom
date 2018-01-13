#include "Node.h"

#include "src/model/schematic/Schematic.h"

using namespace AxiomModel;

Node::Node(Schematic *parent) : GridItem(parent), surface(this) {

}

void Node::setName(const QString &name) {
    if (name != m_name) {
        m_name = name;
        emit nameChanged(name);
    }
}

void Node::setCorners(QPoint topLeft, QPoint bottomRight) {
    auto initialPos = pos();

    // prevent resizing to be smaller than controls
    for (auto &item : surface.items()) {
        auto itemTopLeft = pos() + NodeSurface::nodeSurfaceToSchematic(item->pos());
        auto itemBottomRight = itemTopLeft + NodeSurface::nodeSurfaceToSchematic(QPoint(item->size().width(), item->size().height()));

        topLeft.setX(qMin(topLeft.x(), itemTopLeft.x()));
        topLeft.setY(qMin(topLeft.y(), itemTopLeft.y()));
        bottomRight.setX(qMax(bottomRight.x(), itemBottomRight.x()));
        bottomRight.setY(qMax(bottomRight.y(), itemBottomRight.y()));
    }

    GridItem::setCorners(topLeft, bottomRight);

    // move controls to remain in same schematic-space position
    auto delta = NodeSurface::schematicToNodeSurface(initialPos - pos());
    for (auto &item : surface.items()) {
        surface.grid.setRect(item->pos(), item->size(), nullptr);
    }
    for (auto &item : surface.items()) {
         item->setPos(item->pos() + delta, false, false);
    }
    for (auto &item : surface.items()) {
        surface.grid.setRect(item->pos(), item->size(), item.get());
    }
}
