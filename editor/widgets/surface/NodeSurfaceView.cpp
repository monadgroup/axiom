#include "NodeSurfaceView.h"

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsSceneWheelEvent>
#include <QtWidgets/QOpenGLWidget>

#include "../windows/MainWindow.h"
#include "NodeSurfaceCanvas.h"
#include "NodeSurfacePanel.h"
#include "editor/model/ModelRoot.h"
#include "editor/model/PoolOperators.h"
#include "editor/model/Project.h"
#include "editor/model/actions/DeleteObjectAction.h"
#include "editor/model/actions/GridItemMoveAction.h"
#include "editor/model/actions/PasteBufferAction.h"
#include "editor/model/objects/Node.h"
#include "editor/model/objects/NodeSurface.h"
#include "editor/model/serialize/ModelObjectSerializer.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeSurfaceView::NodeSurfaceView(NodeSurfacePanel *panel, NodeSurface *surface)
    : QGraphicsView(new NodeSurfaceCanvas(panel, surface)), window(panel->window), surface(surface) {
    scene()->setParent(this);
    // setViewport(new QOpenGLWidget());
    setAcceptDrops(true);

    surface->panChanged.connectTo(this, &NodeSurfaceView::pan);
    surface->zoomChanged.connectTo(this, &NodeSurfaceView::zoom);

    // set properties
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);

    setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);
    pan(surface->pan());
    zoom(surface->zoom());

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // connect to global actions
    connect(&window->editUndoAction, &QAction::triggered, this, &NodeSurfaceView::doUndo);
    connect(&window->editRedoAction, &QAction::triggered, this, &NodeSurfaceView::doRedo);
    connect(&window->editDeleteAction, &QAction::triggered, this, &NodeSurfaceView::deleteSelected);
    connect(&window->editSelectAllAction, &QAction::triggered, this, &NodeSurfaceView::selectAll);
    connect(&window->editCutAction, &QAction::triggered, this, &NodeSurfaceView::cutSelected);
    connect(&window->editCopyAction, &QAction::triggered, this, &NodeSurfaceView::copySelected);
    connect(&window->editPasteAction, &QAction::triggered, this, &NodeSurfaceView::pasteBuffer);

    // connect to update history
    surface->root()->history().stackChanged.connectTo(this, &NodeSurfaceView::updateHistoryState);
}

void NodeSurfaceView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning = true;
        startMousePos = event->pos();
        startPan = surface->pan();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }

    QGraphicsView::mousePressEvent(event);
}

void NodeSurfaceView::mouseMoveEvent(QMouseEvent *event) {
    if (isPanning) {
        auto mouseDelta = event->pos() - startMousePos;
        surface->setPan(startPan - mouseDelta / lastScale);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void NodeSurfaceView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning = false;
        QApplication::restoreOverrideCursor();
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void NodeSurfaceView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    pan(surface->pan());
}

void NodeSurfaceView::wheelEvent(QWheelEvent *event) {
    event->ignore();

    QGraphicsSceneWheelEvent wheelEvent(QEvent::GraphicsSceneWheel);
    wheelEvent.setWidget(viewport());
    wheelEvent.setScenePos(mapToScene(event->pos()));
    wheelEvent.setScreenPos(event->globalPos());
    wheelEvent.setButtons(event->buttons());
    wheelEvent.setModifiers(event->modifiers());
    wheelEvent.setDelta(event->delta());
    wheelEvent.setOrientation(event->orientation());
    wheelEvent.setAccepted(false);
    QApplication::sendEvent(scene(), &wheelEvent);
    event->setAccepted(wheelEvent.isAccepted());

    if (!event->isAccepted()) {
        auto translatedEventPos = event->posF() - QPointF(size().width(), size().height()) / 2;
        auto lastScaledPan = translatedEventPos / zoomToScale(surface->zoom());
        auto delta = event->delta() / 1200.f;
        surface->setZoom(surface->zoom() + delta);
        surface->setPan(surface->pan() - translatedEventPos / zoomToScale(surface->zoom()) + lastScaledPan);
    }
}

void NodeSurfaceView::dragEnterEvent(QDragEnterEvent *event) {
    if (!event->mimeData()->hasFormat("application/axiom-partial-surface")) return;

    event->acceptProposedAction();

    // add the nodes to the surface, select them, and make them follow the mouse
    auto scenePos = mapToScene(event->pos());
    auto nodePos = QPoint((int) (scenePos.x() / NodeSurfaceCanvas::nodeGridSize.width()),
                          (int) (scenePos.y() / NodeSurfaceCanvas::nodeGridSize.height()));

    auto data = event->mimeData()->data("application/axiom-partial-surface");
    auto action = PasteBufferAction::create(surface->uuid(), std::move(data), nodePos, surface->root());

    action->forward(true);

    std::vector<std::unique_ptr<Action>> actions;
    actions.push_back(std::move(action));
    dragAndDropAction = CompositeAction::create(std::move(actions), surface->root());

    surface->grid().startDragging();
    startMousePos = QPoint(scenePos.x(), scenePos.y());
}

void NodeSurfaceView::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
    auto mouseDelta = mapToScene(event->pos()) - startMousePos;
    surface->grid().dragTo(QPoint(mouseDelta.x() / NodeSurfaceCanvas::nodeGridSize.width(),
                                  mouseDelta.y() / NodeSurfaceCanvas::nodeGridSize.height()));
}

void NodeSurfaceView::dragLeaveEvent(QDragLeaveEvent *event) {
    surface->grid().finishDragging();
    dragAndDropAction->backward();
    dragAndDropAction.reset();
}

void NodeSurfaceView::dropEvent(QDropEvent *event) {
    event->acceptProposedAction();
    surface->grid().finishDragging();

    auto selectedNodes = AxiomCommon::staticCast<Node *>(surface->grid().selectedItems().sequence());
    for (const auto &selectedNode : selectedNodes) {
        auto beforePos = selectedNode->dragStartPos();
        auto afterPos = selectedNode->pos();

        if (beforePos != afterPos) {
            dragAndDropAction->actions().push_back(
                GridItemMoveAction::create(selectedNode->uuid(), beforePos, afterPos, surface->root()));
        }
    }

    surface->root()->history().append(std::move(dragAndDropAction), false);
    setFocus(Qt::OtherFocusReason);
}

void NodeSurfaceView::focusInEvent(QFocusEvent *event) {
    updateHistoryState();
    QGraphicsView::focusInEvent(event);
}

void NodeSurfaceView::pan(QPointF pan) {
    centerOn(pan);
}

void NodeSurfaceView::zoom(float zoom) {
    auto newScale = zoomToScale(zoom);
    auto scaleChange = newScale / lastScale;
    lastScale = newScale;
    scale(scaleChange, scaleChange);
}

void NodeSurfaceView::deleteSelected() {
    if (!hasFocus()) return;

    std::vector<std::unique_ptr<Action>> deleteActions;
    auto selectedNodes = filter(surface->nodes().sequence(), [](Node *const &node) { return node->isSelected(); });
    for (const auto &node : selectedNodes) {
        if (node->isDeletable()) {
            deleteActions.push_back(DeleteObjectAction::create(node->uuid(), node->root()));
        }
    }

    if (!deleteActions.empty()) {
        surface->root()->history().append(CompositeAction::create(std::move(deleteActions), surface->root()));
    }
}

void NodeSurfaceView::selectAll() {
    if (!hasFocus()) return;

    surface->grid().selectAll();
}

void NodeSurfaceView::cutSelected() {
    if (!hasFocus()) return;

    copySelected();
    deleteSelected();
}

void NodeSurfaceView::copySelected() {
    if (!hasFocus() || surface->grid().selectedItems().sequence().empty()) return;

    auto centerPos = AxiomModel::GridSurface::findCenter(surface->grid().selectedItems().sequence());
    QByteArray serializeArray;
    QDataStream stream(&serializeArray, QIODevice::WriteOnly);
    stream << centerPos;
    ModelObjectSerializer::serializeChunk(stream, surface->uuid(), surface->getCopyItems());

    auto mimeData = new QMimeData();
    mimeData->setData("application/axiom-partial-surface", serializeArray);
    auto clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

void NodeSurfaceView::pasteBuffer() {
    if (!hasFocus()) return;

    auto mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData || !mimeData->hasFormat("application/axiom-partial-surface")) return;

    auto buffer = mimeData->data("application/axiom-partial-surface");
    auto scenePos = mapToScene(mapFromGlobal(QCursor::pos()));
    auto targetPos = QPoint(qRound((float) scenePos.x() / NodeSurfaceCanvas::nodeGridSize.width()),
                            qRound((float) scenePos.y() / NodeSurfaceCanvas::nodeGridSize.height()));
    surface->root()->history().append(
        PasteBufferAction::create(surface->uuid(), std::move(buffer), targetPos, surface->root()));
}

void NodeSurfaceView::doUndo() {
    if (hasFocus()) {
        surface->root()->history().undo();
    }
}

void NodeSurfaceView::doRedo() {
    if (hasFocus()) {
        surface->root()->history().redo();
    }
}

float NodeSurfaceView::zoomToScale(float zoom) {
    return std::pow(20.f, zoom);
}

void NodeSurfaceView::updateHistoryState() {
    if (!hasFocus()) return;
    window->editUndoAction.setText("&Undo " + AxiomModel::Action::typeToString(surface->root()->history().undoType()));
    window->editRedoAction.setText("&Redo " + AxiomModel::Action::typeToString(surface->root()->history().redoType()));
    window->editUndoAction.setEnabled(surface->root()->history().canUndo());
    window->editRedoAction.setEnabled(surface->root()->history().canRedo());
}
