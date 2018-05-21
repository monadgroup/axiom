#include "NodeSurfaceView.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsItem>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsSceneWheelEvent>

#include "editor/model/objects/NodeSurface.h"
#include "editor/model/Project.h"
#include "NodeSurfaceCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeSurfaceView::NodeSurfaceView(NodeSurfacePanel *panel, NodeSurface *surface)
    : QGraphicsView(new NodeSurfaceCanvas(panel, surface)), surface(surface) {
    scene()->setParent(this);
    setAcceptDrops(true);

    surface->panChanged.connect(this, &NodeSurfaceView::pan);
    surface->zoomChanged.connect(this, &NodeSurfaceView::zoom);

    // set properties
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);

    setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);
    pan(surface->pan());
    zoom(surface->zoom());

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
    /*if (!event->mimeData()->hasFormat("application/axiom-partial-surface")) return;

    event->acceptProposedAction();
    schematic->project()->history.startAction(HistoryList::ActionType::PLACE_MODULE);

    auto currentItemCount = schematic->items().size();

    // add the nodes to the surface, select them, and make them follow the mouse
    auto scenePos = mapToScene(event->pos());
    auto nodePos = QPoint(
        (int) (scenePos.x() / SchematicCanvas::nodeGridSize.width()),
        (int) (scenePos.y() / SchematicCanvas::nodeGridSize.height())
    );

    auto data = event->mimeData()->data("application/axiom-partial-surface");
    QDataStream stream(&data, QIODevice::ReadOnly);
    schematic->partialDeserialize(stream, nodePos);

    schematic->deselectAll();
    for (size_t i = currentItemCount; i < schematic->items().size(); i++) {
        schematic->items()[i]->select(false);
    }
    schematic->startDragging();
    startMousePos = QPoint(scenePos.x(), scenePos.y());*/
}

void NodeSurfaceView::dragMoveEvent(QDragMoveEvent *event) {
    /*auto mouseDelta = mapToScene(event->pos()) - startMousePos;
    surface->dragTo(QPoint(
        mouseDelta.x() / SchematicCanvas::nodeGridSize.width(),
        mouseDelta.y() / SchematicCanvas::nodeGridSize.height()
    ));*/
}

void NodeSurfaceView::dragLeaveEvent(QDragLeaveEvent *event) {
    //schematic->finishDragging();
    //schematic->project()->history.cancelAction(HistoryList::ActionType::PLACE_MODULE);
}

void NodeSurfaceView::dropEvent(QDropEvent *event) {
    //schematic->finishDragging();
    //schematic->project()->history.endAction(HistoryList::ActionType::PLACE_MODULE);
    //setFocus(Qt::OtherFocusReason);
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

float NodeSurfaceView::zoomToScale(float zoom) {
    return std::pow(20.f, zoom);
}
