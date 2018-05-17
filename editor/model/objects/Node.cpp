#include "Node.h"

#include "NodeSurface.h"
#include "ControlSurface.h"
#include "Control.h"
#include "CustomNode.h"
#include "GroupNode.h"
#include "PortalNode.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

Node::Node(NodeType nodeType, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
           QString name, const QUuid &controlsUuid, AxiomModel::ModelRoot *root)
    : GridItem(&find(root->nodeSurfaces(), parentUuid)->grid(), pos, size, selected),
      ModelObject(ModelType::NODE, uuid, parentUuid, root), _nodeType(nodeType), _name(std::move(name)),
      _controls(findLater<ControlSurface *>(root->controlSurfaces(), controlsUuid)) {
}

std::unique_ptr<Node> Node::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                        AxiomModel::ModelRoot *root) {
    uint8_t nodeTypeInt;
    stream >> nodeTypeInt;

    QPoint pos;
    QSize size;
    bool selected;
    GridItem::deserialize(stream, pos, size, selected);

    QString name;
    stream >> name;
    QUuid controlsUuid;
    stream >> controlsUuid;

    switch ((NodeType) nodeTypeInt) {
        case NodeType::CUSTOM_NODE:
            return CustomNode::deserialize(stream, uuid, parentUuid, pos, size, selected, name, controlsUuid, root);
        case NodeType::GROUP_NODE:
            return GroupNode::deserialize(stream, uuid, parentUuid, pos, size, selected, name, controlsUuid, root);
        case NodeType::PORTAL_NODE:
            return PortalNode::deserialize(stream, uuid, parentUuid, pos, size, selected, name, controlsUuid, root);
    }

    unreachable;
}

void Node::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    ModelObject::serialize(stream, parent, withContext);

    stream << (uint8_t) _nodeType;
    GridItem::serialize(stream);
    stream << _name;
    stream << (*_controls.value())->uuid();
}

void Node::setName(const QString &name) {
    if (name != _name) {
        _name = name;
        nameChanged.trigger(name);
    }
}

void Node::setExtracted(bool extracted) {
    if (extracted != _isExtracted) {
        _isExtracted = extracted;
        extractedChanged.trigger(extracted);
    }
}

void Node::setCorners(QPoint topLeft, QPoint bottomRight) {
    if (!_controls.value()) {
        return GridItem::setCorners(topLeft, bottomRight);
    }
    auto controlSurface = *_controls.value();

    auto initialPos = pos();
    auto initialBottomRight = initialPos + QPoint(size().width(), size().height());

    // calculate controls bounding region
    auto controlsTopLeft = pos() + QPoint(size().width(), size().height());
    auto controlsBottomRight = pos();
    for (auto &item : controlSurface->controls()) {
        auto itemTopLeft = pos() + ControlSurface::controlToNodeFloor(item->pos());
        auto itemBottomRight = pos() + ControlSurface::controlToNodeCeil(
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
    auto delta = ControlSurface::nodeToControl(initialPos - pos() + controlsShift);
    for (auto &item : controlSurface->controls()) {
        controlSurface->grid().grid().setRect(item->pos(), item->size(), nullptr);
    }
    for (auto &item : controlSurface->controls()) {
        item->setPos(item->pos() + delta, false, false);
    }
    for (auto &item : controlSurface->controls()) {
        controlSurface->grid().grid().setRect(item->pos(), item->size(), item);
    }
    controlSurface->grid().flushGrid();
}

void Node::remove() {
    if (_controls.value()) (*_controls.value())->remove();
    ModelObject::remove();
}
