#include "NodeItem.h"

#include <QtCore/QTimer>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "../FloatingValueEditor.h"
#include "../ItemResizer.h"
#include "../surface/NodeSurfaceCanvas.h"
#include "../surface/NodeSurfacePanel.h"
#include "../windows/MainWindow.h"
#include "../windows/ModulePropertiesWindow.h"
#include "CustomNodePanel.h"
#include "editor/model/CloneReferenceMapper.h"
#include "editor/model/Library.h"
#include "editor/model/LibraryEntry.h"
#include "editor/model/PoolOperators.h"
#include "editor/model/actions/CompositeAction.h"
#include "editor/model/actions/DeleteObjectAction.h"
#include "editor/model/actions/GridItemMoveAction.h"
#include "editor/model/actions/GridItemSizeAction.h"
#include "editor/model/actions/RenameNodeAction.h"
#include "editor/model/objects/ControlSurface.h"
#include "editor/model/objects/CustomNode.h"
#include "editor/model/objects/ExtractControl.h"
#include "editor/model/objects/GraphControl.h"
#include "editor/model/objects/GroupNode.h"
#include "editor/model/objects/MidiControl.h"
#include "editor/model/objects/Node.h"
#include "editor/model/objects/NumControl.h"
#include "editor/model/objects/PortalControl.h"
#include "editor/model/objects/PortalNode.h"
#include "editor/model/objects/RootSurface.h"
#include "editor/model/serialize/ModelObjectSerializer.h"
#include "editor/model/serialize/ProjectSerializer.h"
#include "editor/widgets/controls/ExtractControlItem.h"
#include "editor/widgets/controls/GraphControlItem.h"
#include "editor/widgets/controls/MidiControlItem.h"
#include "editor/widgets/controls/NumControlItem.h"
#include "editor/widgets/controls/PortalControlItem.h"

using namespace AxiomGui;
using namespace AxiomModel;

const qreal CONTROL_ZVALUE = 1;
const qreal EDGE_RESIZE_ZVALUE = 2;
const qreal CORNER_RESIZE_ZVALUE = 3;

NodeItem::NodeItem(Node *node, NodeSurfaceCanvas *canvas) : canvas(canvas), node(node) {
    node->nameChanged.connect(this, &NodeItem::triggerUpdate);
    node->extractedChanged.connect(this, &NodeItem::triggerUpdate);
    node->posChanged.connect(this, &NodeItem::setPos);
    node->beforeSizeChanged.connect(this, &NodeItem::triggerGeometryChange);
    node->sizeChanged.connect(this, &NodeItem::setSize);
    node->selectedChanged.connect(this, &NodeItem::setIsSelected);
    node->deselected.connect(this, &NodeItem::triggerUpdate);
    node->removed.connect(this, &NodeItem::remove);

    node->controls().then([this](ControlSurface *surface) {
        surface->controlsOnTopRowChanged.connect(this, &NodeItem::triggerUpdate);
        surface->grid().hasSelectionChanged.connect(this, &NodeItem::triggerUpdate);
    });

    // create resize items
    if (node->isResizable()) {
        ItemResizer::Direction directions[] = {
            ItemResizer::TOP,       ItemResizer::RIGHT,    ItemResizer::BOTTOM,       ItemResizer::LEFT,
            ItemResizer::TOP_RIGHT, ItemResizer::TOP_LEFT, ItemResizer::BOTTOM_RIGHT, ItemResizer::BOTTOM_LEFT};
        for (auto i = 0; i < 8; i++) {
            auto resizer = new ItemResizer(directions[i],
                                           QSize(NodeSurfaceCanvas::nodeGridSize.width() * node->minSize().width(),
                                                 NodeSurfaceCanvas::nodeGridSize.height() * node->minSize().height()));

            // ensure corners are on top of edges
            resizer->setZValue(i > 3 ? CORNER_RESIZE_ZVALUE : EDGE_RESIZE_ZVALUE);

            connect(this, &NodeItem::resizerPosChanged, resizer, &ItemResizer::setPos);
            connect(this, &NodeItem::resizerSizeChanged, resizer, &ItemResizer::setSize);

            connect(resizer, &ItemResizer::startDrag, this, &NodeItem::resizerStartDrag);
            connect(resizer, &ItemResizer::changed, this, &NodeItem::resizerChanged);
            connect(resizer, &ItemResizer::endDrag, this, &NodeItem::resizerEndDrag);

            node->controls().then([resizer](ControlSurface *surface) {
                resizer->setVisible(!surface->grid().hasSelection());
                surface->grid().hasSelectionChanged.connect(
                    [resizer](bool hasSelection) { resizer->setVisible(!hasSelection); });
            });

            resizer->setParentItem(this);
        }
    }

    if (auto customNode = dynamic_cast<CustomNode *>(node)) {
        auto panel = new CustomNodePanel(customNode);
        panel->setParentItem(this);
        panel->setFlag(QGraphicsItem::ItemStacksBehindParent);
    }

    // set initial state
    setPos(node->pos());
    setSize(node->size());
    setIsSelected(node->isSelected());

    // create items for all controls that already exist
    node->controls().then([this](ControlSurface *surface) {
        for (const auto &control : surface->controls()) {
            addControl(control);
        }

        surface->controls().itemAdded.connect(this, &NodeItem::addControl);
        surface->grid().hasSelectionChanged.connect(this, &NodeItem::triggerUpdate);
    });
}

QRectF NodeItem::boundingRect() const {
    auto drawBr = drawBoundingRect();
    return {drawBr.topLeft() - QPointF(0, textOffset), drawBr.size() + QSizeF(0, textOffset)};
}

QRectF NodeItem::drawBoundingRect() const {
    return {QPointF(0, 0), NodeSurfaceCanvas::nodeRealSize(node->size())};
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    QColor darkColor, lightColor, outlineColor;
    switch (node->nodeType()) {
    case Node::NodeType::PORTAL_NODE:
    case Node::NodeType::CUSTOM_NODE:
        darkColor = CommonColors::customNodeNormal;
        lightColor = CommonColors::customNodeSelected;
        outlineColor = CommonColors::customNodeBorder;
        break;
    case Node::NodeType::GROUP_NODE:
        darkColor = CommonColors::groupNodeNormal;
        lightColor = CommonColors::groupNodeSelected;
        outlineColor = CommonColors::groupNodeBorder;
        break;
    }

    painter->setPen(QPen(outlineColor, node->isExtracted() ? 3 : 1));
    if (node->isSelected()) {
        painter->setBrush(QBrush(lightColor));
    } else {
        painter->setBrush(QBrush(darkColor));
    }
    painter->drawRect(drawBoundingRect());

    auto gridPen = QPen(QColor(lightColor.red(), lightColor.green(), lightColor.blue(), 255), 1);

    if (node->controls().value() && (*node->controls().value())->grid().hasSelection()) {
        painter->setPen(QPen(outlineColor, 1));
        auto nodeSurfaceSize = ControlSurface::nodeToControl(node->size());
        for (auto x = 0; x < nodeSurfaceSize.width(); x++) {
            for (auto y = 0; y < nodeSurfaceSize.height(); y++) {
                painter->drawPoint(NodeSurfaceCanvas::controlRealPos(QPoint(x, y)));
            }
        }
    }

    if ((*node->controls().value())->controlsOnTopRow()) {
        painter->setPen(QPen(QColor(100, 100, 100)));
        auto br = boundingRect();
        auto textBound = QRectF(br.topLeft(), QSizeF(br.width(), textOffset));
        painter->drawText(textBound, Qt::AlignLeft | Qt::AlignTop, node->name());
    } else {
        painter->setPen(Qt::NoPen);
        painter->setBrush(node->isSelected() ? outlineColor : lightColor);
        auto headerBr = drawBoundingRect();
        headerBr.setHeight(NodeSurfaceCanvas::controlGridSize.height());
        headerBr.setTopLeft(headerBr.topLeft() + QPointF(1, 1));
        painter->drawRect(headerBr);
        painter->setPen(QColor(200, 200, 200));
        headerBr.setLeft(headerBr.left() + 8);
        painter->drawText(headerBr, Qt::AlignLeft | Qt::AlignVCenter, node->name());
    }
}

QPainterPath NodeItem::shape() const {
    QPainterPath path;
    path.addRect(drawBoundingRect());
    return path;
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (node->controls().value()) (*node->controls().value())->grid().deselectAll();

    if (event->button() == Qt::LeftButton) {
        if (!node->isSelected()) node->select(!(event->modifiers() & Qt::ShiftModifier));

        isDragging = true;
        mouseStartPoint = event->screenPos();
        node->startedDragging.trigger();
    }

    event->accept();
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->screenPos() - mouseStartPoint;
    node->draggedTo.trigger(QPoint(qRound((float) mouseDelta.x() / NodeSurfaceCanvas::nodeGridSize.width()),
                                   qRound((float) mouseDelta.y() / NodeSurfaceCanvas::nodeGridSize.height())));

    event->accept();
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;
    isDragging = false;
    node->finishedDragging.trigger();

    std::vector<std::unique_ptr<Action>> dragEvents;
    auto selectedNodes = staticCast<Node *>(node->parentSurface->selectedItems().sequence());
    for (const auto &selectedNode : selectedNodes) {
        auto beforePos = selectedNode->dragStartPos();
        auto afterPos = selectedNode->pos();

        if (beforePos != afterPos) {
            dragEvents.push_back(GridItemMoveAction::create(selectedNode->uuid(), beforePos, afterPos, node->root()));
        }
    }

    if (!dragEvents.empty()) {
        node->root()->history().append(CompositeAction::create(std::move(dragEvents), node->root()));
    }

    event->accept();
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (auto groupNode = dynamic_cast<GroupNode *>(node); groupNode && groupNode->nodes().value()) {
        event->accept();
        canvas->panel->window->showSurface(canvas->panel, *groupNode->nodes().value(), false, false);
    } else if (auto customNode = dynamic_cast<CustomNode *>(node)) {
        event->accept();
        customNode->setPanelOpen(!customNode->isPanelOpen());
    }
}

void NodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    if (!node->isSelected()) node->select(true);

    event->accept();

    auto copyableItems =
        AxiomModel::filter(AxiomModel::findChildren(node->root()->nodes().sequence(), node->parentUuid()),
                           [](Node *const &node) { return node->isCopyable(); });

    QMenu menu;

    auto renameAction = menu.addAction(tr("&Rename..."));
    renameAction->setVisible(node->parentSurface->selectedItems().size() == 1);
    menu.addSeparator();
    auto groupAction = menu.addAction(tr("&Group..."));
    groupAction->setEnabled(!copyableItems.empty());
    auto saveModuleAction = menu.addAction(tr("&Save as Module..."));
    saveModuleAction->setEnabled(!copyableItems.empty());
    menu.addSeparator();

    QAction *fiddleAction = nullptr;
    auto rootSurface = dynamic_cast<RootSurface *>(node->surface());
    auto mainWindow = canvas->panel->window;
    PortalControl *portalControl = nullptr;
    if (auto portalNode = dynamic_cast<PortalNode *>(node); portalNode && rootSurface && rootSurface->compileMeta() &&
                                                            mainWindow->project()->backend()->canFiddleAutomation()) {
        portalControl =
            AxiomModel::takeAt(dynamicCast<PortalControl *>((*portalNode->controls().value())->controls()), 0);
        if (portalControl->portalType() == PortalControl::PortalType::AUTOMATION) {
            fiddleAction = menu.addAction(tr("&Fiddle"));
            menu.addSeparator();
        }
    }

    auto deleteAction = menu.addAction(tr("&Delete"));
    deleteAction->setEnabled(node->isDeletable());
    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == renameAction) {
        auto editor = new FloatingValueEditor(node->name(), event->scenePos());
        scene()->addItem(editor);

        connect(editor, &FloatingValueEditor::valueSubmitted, this, [this](QString name) {
            node->root()->history().append(RenameNodeAction::create(node->uuid(), node->name(), name, node->root()));
        });
    } else if (selectedAction == saveModuleAction) {
        ModulePropertiesWindow saveWindow(mainWindow->library());
        if (saveWindow.exec() == QDialog::Accepted) {
            auto enteredName = saveWindow.enteredName();
            auto enteredTags = saveWindow.enteredTags();

            auto newEntry =
                LibraryEntry::create(std::move(enteredName), std::set<QString>(enteredTags.begin(), enteredTags.end()));
            auto centerPos = AxiomModel::GridSurface::findCenter(node->surface()->grid().selectedItems());
            QByteArray serializeArray;
            QDataStream serializeStream(&serializeArray, QIODevice::WriteOnly);
            ModelObjectSerializer::serializeChunk(serializeStream, node->surface()->uuid(),
                                                  node->surface()->getCopyItems());

            QDataStream deserializeStream(&serializeArray, QIODevice::ReadOnly);
            AxiomModel::CloneReferenceMapper ref;
            ref.setUuid(newEntry->rootSurface()->uuid(), newEntry->rootSurface()->uuid());
            ref.setPos(newEntry->rootSurface()->uuid(), -centerPos);
            ModelObjectSerializer::deserializeChunk(deserializeStream, ProjectSerializer::schemaVersion,
                                                    newEntry->root(), newEntry->rootSurface()->uuid(), &ref);

            mainWindow->library()->addEntry(std::move(newEntry));
        }
    } else if (selectedAction == deleteAction) {
        node->root()->history().append(DeleteObjectAction::create(node->uuid(), node->root()));
    } else if (selectedAction == fiddleAction && portalControl) {
        auto backend = mainWindow->project()->backend();
        auto remappedIndex = backend->internalRemapPortal(portalControl->portalId());
        auto currentPortalValue = **backend->getAudioPortal(remappedIndex);
        backend->automationValueChanged(remappedIndex, currentPortalValue);
    }
}

void NodeItem::setPos(QPoint newPos) {
    auto realPos = NodeSurfaceCanvas::nodeRealPos(newPos);
    QGraphicsItem::setPos(realPos.x(), realPos.y());
    emit resizerPosChanged(realPos);
}

void NodeItem::setSize(QSize newSize) {
    emit resizerSizeChanged(NodeSurfaceCanvas::nodeRealSize(newSize));
}

void NodeItem::addControl(Control *control) {
    ControlItem *item = nullptr;

    if (auto numControl = dynamic_cast<NumControl *>(control)) {
        item = new NumControlItem(numControl, canvas);
    } else if (auto midiControl = dynamic_cast<MidiControl *>(control)) {
        item = new MidiControlItem(midiControl, canvas);
    } else if (auto extractControl = dynamic_cast<ExtractControl *>(control)) {
        item = new ExtractControlItem(extractControl, canvas);
    } else if (auto outputControl = dynamic_cast<PortalControl *>(control)) {
        item = new PortalControlItem(outputControl, canvas);
    } else if (auto graphControl = dynamic_cast<GraphControl *>(control)) {
        item = new GraphControlItem(graphControl, canvas);
    }

    assert(item);
    item->setZValue(CONTROL_ZVALUE);
    item->setParentItem(this);
}

void NodeItem::setIsSelected(bool selected) {
    setZValue(selected ? NodeSurfaceCanvas::activeNodeZVal : NodeSurfaceCanvas::nodeZVal);
}

void NodeItem::remove() {
    scene()->removeItem(this);
}

void NodeItem::resizerChanged(QPointF topLeft, QPointF bottomRight) {
    node->setCorners(QPoint(qRound(topLeft.x() / NodeSurfaceCanvas::nodeGridSize.width()),
                            qRound(topLeft.y() / NodeSurfaceCanvas::nodeGridSize.height())),
                     QPoint(qRound(bottomRight.x() / NodeSurfaceCanvas::nodeGridSize.width()),
                            qRound(bottomRight.y() / NodeSurfaceCanvas::nodeGridSize.height())));
}

void NodeItem::resizerStartDrag() {
    node->select(true);
    node->startSize();
}

void NodeItem::resizerEndDrag() {
    node->doSizeAction();
}

void NodeItem::triggerUpdate() {
    update();
}

void NodeItem::triggerGeometryChange() {
    prepareGeometryChange();
}
