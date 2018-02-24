#include "Node.h"

#include "../schematic/Schematic.h"
#include "../control/NodeControl.h"

using namespace AxiomModel;

Node::Node(Schematic *parent, QString name, Type type, QPoint pos, QSize size)
    : GridItem(parent, pos, size), parentSchematic(parent), m_name(std::move(name)), type(type), surface(this) {
    connect(this, &Node::deselected,
            &surface, &NodeSurface::deselectAll);
}

void Node::doRuntimeUpdate() {
    surface.doRuntimeUpdate();
}

void Node::setName(const QString &name) {
    if (name != m_name) {
        m_name = name;
        emit nameChanged(name);
    }
}

void Node::setPanelOpen(bool panelOpen) {
    if (panelOpen != m_panelOpen) {
        m_panelOpen = panelOpen;
        emit panelOpenChanged(panelOpen);
    }
}

void Node::setPanelHeight(float panelHeight) {
    if (panelHeight < minPanelHeight) panelHeight = minPanelHeight;
    if (panelHeight != m_panelHeight) {
        emit beforePanelHeightChanged(panelHeight);
        m_panelHeight = panelHeight;
        emit panelHeightChanged(panelHeight);
    }
}

void Node::setCorners(QPoint topLeft, QPoint bottomRight) {
    auto initialPos = pos();
    auto initialBottomRight = initialPos + QPoint(size().width(), size().height());

    // calculate controls bounding region
    auto controlsTopLeft = pos() + QPoint(size().width(), size().height());
    auto controlsBottomRight = pos();
    for (auto &item : surface.items()) {
        auto itemTopLeft = pos() + NodeSurface::nodeSurfaceToSchematicFloor(item->pos());
        auto itemBottomRight = pos() + NodeSurface::nodeSurfaceToSchematicCeil(
            item->pos() + QPoint(item->size().width(), item->size().height()));

        controlsTopLeft.setX(qMin(controlsTopLeft.x(), itemTopLeft.x()));
        controlsTopLeft.setY(qMin(controlsTopLeft.y(), itemTopLeft.y()));
        controlsBottomRight.setX(qMax(controlsBottomRight.x(), itemBottomRight.x()));
        controlsBottomRight.setY(qMax(controlsBottomRight.y(), itemBottomRight.y()));
    }

    // find max top left where we can still fit the controls
    auto controlsSize = controlsBottomRight - controlsTopLeft;

    auto fitTopLeft = bottomRight - controlsSize;

    if (topLeft.x() != initialPos.x()) topLeft.setX(qMin(topLeft.x(), fitTopLeft.x()));
    if (topLeft.y() != initialPos.y()) topLeft.setY(qMin(topLeft.y(), fitTopLeft.y()));

    // find min bottom right where we can still fit the controls
    auto fitBottomRight = topLeft + controlsSize;

    if (bottomRight.x() != initialBottomRight.x()) bottomRight.setX(qMax(bottomRight.x(), fitBottomRight.x()));
    if (bottomRight.y() != initialBottomRight.y()) bottomRight.setY(qMax(bottomRight.y(), fitBottomRight.y()));

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
    surface.flushGrid();
}
