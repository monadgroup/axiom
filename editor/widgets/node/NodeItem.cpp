#include "NodeItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtCore/QTimer>

#include "editor/widgets/ItemResizer.h"
#include "../schematic/SchematicCanvas.h"
#include "editor/model/node/Node.h"
#include "editor/model/node/CustomNode.h"
#include "editor/model/node/ModuleNode.h"
#include "editor/model/control/NodeControl.h"
#include "editor/model/control/NodeNumControl.h"
#include "editor/widgets/controls/NumControl.h"
#include "../schematic/SchematicPanel.h"
#include "../windows/MainWindow.h"
#include "../FloatingValueEditor.h"
#include "../CommonColors.h"
#include "CustomNodePanel.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItem::NodeItem(Node *node, SchematicCanvas *canvas) : node(node), canvas(canvas) {
    // create items for all controls that already exist
    for (const auto &item : node->surface.items()) {
        if (auto control = dynamic_cast<NodeControl *>(item.get())) {
            addControl(control);
        }
    }

    connect(node, &Node::nameChanged,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::posChanged,
            this, &NodeItem::setPos);
    connect(node, &Node::beforeSizeChanged,
            this, &NodeItem::triggerGeometryChange);
    connect(node, &Node::sizeChanged,
            this, &NodeItem::setSize);
    connect(node, &Node::selectedChanged,
            this, &NodeItem::setIsSelected);
    connect(node, &Node::deselected,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::removed,
            this, &NodeItem::remove);

    connect(&node->surface, &NodeSurface::hasSelectionChanged,
            this, &NodeItem::triggerUpdate);

    connect(&node->surface, &NodeSurface::itemAdded,
            [this](AxiomModel::GridItem *item) {
                if (auto control = dynamic_cast<NodeControl *>(item)) {
                    addControl(control);
                }
            });

    // create resize items
    ItemResizer::Direction directions[] = {
            ItemResizer::TOP, ItemResizer::RIGHT, ItemResizer::BOTTOM, ItemResizer::LEFT,
            ItemResizer::TOP_RIGHT, ItemResizer::TOP_LEFT, ItemResizer::BOTTOM_RIGHT, ItemResizer::BOTTOM_LEFT
    };
    for (auto i = 0; i < 8; i++) {
        auto resizer = new ItemResizer(directions[i], SchematicCanvas::nodeGridSize);

        // ensure corners are on top of edges
        resizer->setZValue(i > 3 ? 2 : 1);

        connect(this, &NodeItem::resizerPosChanged,
                resizer, &ItemResizer::setPos);
        connect(this, &NodeItem::resizerSizeChanged,
                resizer, &ItemResizer::setSize);

        connect(resizer, &ItemResizer::startDrag,
                this, &NodeItem::resizerStartDrag);
        connect(resizer, &ItemResizer::changed,
                this, &NodeItem::resizerChanged);

        connect(&node->surface, &NodeSurface::hasSelectionChanged,
                [this, resizer](auto hasSelection) { resizer->setVisible(!hasSelection); });

        resizer->setParentItem(this);
        resizer->setVisible(!node->surface.hasSelection());
    }

    if (auto customNode = dynamic_cast<CustomNode*>(node)) {
        auto panel = new CustomNodePanel(customNode);
        panel->setParentItem(this);
        panel->setFlag(QGraphicsItem::ItemStacksBehindParent);
    }

    // set initial state
    setPos(node->pos());
    setSize(node->size());
    setIsSelected(node->isSelected());
}

QRectF NodeItem::boundingRect() const {
    auto drawBr = drawBoundingRect();
    return {
            drawBr.topLeft() - QPointF(0, textOffset),
            drawBr.size() + QSizeF(0, textOffset)
    };
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    QColor darkColor, lightColor, outlineColor;
    switch (node->type) {
        case Node::Type::CUSTOM:
            darkColor = CommonColors::customNodeNormal;
            lightColor = CommonColors::customNodeActive;
            outlineColor = CommonColors::customNodeBorder;
            break;
        case Node::Type::GROUP:
            darkColor = CommonColors::groupNodeNormal;
            lightColor = CommonColors::groupNodeActive;
            outlineColor = CommonColors::groupNodeBorder;
            break;
    }

    painter->setPen(QPen(outlineColor, 1));
    if (node->isSelected()) {
        painter->setBrush(QBrush(lightColor));
    } else {
        painter->setBrush(QBrush(darkColor));
    }
    painter->drawRect(drawBoundingRect());

    auto gridPen = QPen(QColor(lightColor.red(), lightColor.green(), lightColor.blue(), 255), 1);

    if (node->surface.hasSelection()) {
        auto nodeSurfaceSize = NodeSurface::schematicToNodeSurface(node->size());
        for (auto x = 0; x < nodeSurfaceSize.width(); x++) {
            for (auto y = 0; y < nodeSurfaceSize.height(); y++) {
                painter->drawPoint(SchematicCanvas::controlRealPos(QPoint(x, y)));
            }
        }
    }

    painter->setPen(QPen(QColor(100, 100, 100)));
    auto br = boundingRect();
    auto textBound = QRectF(
            br.topLeft(),
            QSizeF(br.width(), textOffset)
    );
    painter->drawText(textBound, Qt::AlignLeft | Qt::AlignTop, node->name());
}

QPainterPath NodeItem::shape() const {
    QPainterPath path;
    path.addRect(drawBoundingRect());
    return path;
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    node->surface.deselectAll();

    if (event->button() == Qt::LeftButton) {
        if (!node->isSelected()) node->select(!(event->modifiers() & Qt::ShiftModifier));

        isDragging = true;
        mouseStartPoint = event->screenPos();
        emit node->startedDragging();
    }

    event->accept();
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->screenPos() - mouseStartPoint;
    emit node->draggedTo(QPoint(
            qRound((float) mouseDelta.x() / SchematicCanvas::nodeGridSize.width()),
            qRound((float) mouseDelta.y() / SchematicCanvas::nodeGridSize.height())
    ));

    event->accept();
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    emit node->finishedDragging();

    event->accept();
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (auto moduleNode = dynamic_cast<ModuleNode *>(node)) {
        event->accept();

        canvas->panel->window->showSchematic(
                canvas->panel,
                moduleNode->schematic.get(),
                false
        );
    } else {
        event->accept();
        node->setPanelOpen(!node->isPanelOpen());
    }
}

void NodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    if (!node->isSelected()) node->select(true);

    event->accept();

    QMenu menu;

    auto renameAction = menu.addAction(tr("&Rename..."));
    renameAction->setVisible(node->parentSchematic->selectedItems().size() == 1);
    menu.addSeparator();
    auto groupAction = menu.addAction(tr("&Group..."));
    auto savePresetAction = menu.addAction(tr("&Save as Preset..."));
    menu.addSeparator();
    auto deleteAction = menu.addAction(tr("&Delete"));
    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == renameAction) {
        auto editor = new FloatingValueEditor(node->name(), event->scenePos());
        scene()->addItem(editor);

        connect(editor, &FloatingValueEditor::valueSubmitted,
                node, &Node::setName);
    } else if (selectedAction == groupAction) {
        auto groupedNode = node->parentSchematic->groupSelection();
        canvas->panel->window->showSchematic(
                canvas->panel,
                groupedNode->schematic.get(),
                true
        );
    } else if (selectedAction == savePresetAction) {
        // todo: save as preset
    } else if (selectedAction == deleteAction) {
        node->parentSurface->deleteSelectedItems();
    }
}

void NodeItem::setPos(QPoint newPos) {
    auto realPos = SchematicCanvas::nodeRealPos(newPos);
    QGraphicsItem::setPos(realPos.x(), realPos.y());
    emit resizerPosChanged(realPos);
}

void NodeItem::setSize(QSize newSize) {
    emit resizerSizeChanged(SchematicCanvas::nodeRealSize(newSize));
}

void NodeItem::addControl(NodeControl *control) {
    if (auto numControl = dynamic_cast<NodeNumControl *>(control)) {
        auto c = new NumControl(numControl, canvas);
        c->setZValue(2);
        c->setParentItem(this);
    }
}

void NodeItem::setIsSelected(bool selected) {
    setZValue(selected ? SchematicCanvas::activeNodeZVal : SchematicCanvas::nodeZVal);
}

void NodeItem::remove() {
    scene()->removeItem(this);
}

void NodeItem::resizerChanged(QPointF topLeft, QPointF bottomRight) {
    node->setCorners(QPoint(
            qRound(topLeft.x() / SchematicCanvas::nodeGridSize.width()),
            qRound(topLeft.y() / SchematicCanvas::nodeGridSize.height())
    ), QPoint(
            qRound(bottomRight.x() / SchematicCanvas::nodeGridSize.width()),
            qRound(bottomRight.y() / SchematicCanvas::nodeGridSize.height())
    ));
}

void NodeItem::resizerStartDrag() {
    node->select(true);
}

void NodeItem::triggerUpdate() {
    update();
}

void NodeItem::triggerGeometryChange() {
    prepareGeometryChange();
}

QRectF NodeItem::drawBoundingRect() const {
    return {
            QPointF(0, 0),
            SchematicCanvas::nodeRealSize(node->size())
    };
}
