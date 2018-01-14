#include "Node.h"

#include "editor/model/schematic/Schematic.h"

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

    // calculate controls bounding region
    auto controlsTopLeft = pos() + QPoint(size().width(), size().height());
    auto controlsBottomRight = pos();
    for (auto &item : surface.items()) {
        auto itemTopLeft = pos() + NodeSurface::nodeSurfaceToSchematicFloor(item->pos());
        auto itemBottomRight = pos() + NodeSurface::nodeSurfaceToSchematicCeil(item->pos() + QPoint(item->size().width(), item->size().height()));

        controlsTopLeft.setX(qMin(controlsTopLeft.x(), itemTopLeft.x()));
        controlsTopLeft.setY(qMin(controlsTopLeft.y(), itemTopLeft.y()));
        controlsBottomRight.setX(qMax(controlsBottomRight.x(), itemBottomRight.x()));
        controlsBottomRight.setY(qMax(controlsBottomRight.y(), itemBottomRight.y()));
    }

    // find max top left where we can still fit the controls
    auto controlsSize = controlsBottomRight - controlsTopLeft;

    if (topLeft != pos()) {
        auto fitTopLeft = bottomRight - controlsSize;
        topLeft.setX(qMin(topLeft.x(), fitTopLeft.x()));
        topLeft.setY(qMin(topLeft.y(), fitTopLeft.y()));
    }

    if (bottomRight != pos() + QPoint(size().width(), size().height())) {
        auto fitBottomRight = topLeft + controlsSize;
        bottomRight.setX(qMax(bottomRight.x(), fitBottomRight.x()));
        bottomRight.setY(qMax(bottomRight.y(), fitBottomRight.y()));
    }

    GridItem::setCorners(topLeft, bottomRight);

    // move controls to remain in same schematic-space position,
    // except when topLeft > controlsTopLeft or bottomRight < controlsBottomRight
    auto controlsShift = QPoint(
            qMax(0, topLeft.x() - controlsTopLeft.x()) + qMin(0, bottomRight.x() - controlsBottomRight.x()),
            qMax(0, topLeft.y() - controlsTopLeft.y()) + qMin(0, bottomRight.y() - controlsBottomRight.y())
    );
    auto delta = NodeSurface::schematicToNodeSurface(initialPos - pos() + controlsShift);
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
