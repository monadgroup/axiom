#include "SchematicView.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsItem>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsSceneWheelEvent>
#include <QtCore/QMimeData>

#include "editor/model/node/CustomNode.h"
#include "editor/model/schematic/Schematic.h"
#include "editor/model/Project.h"
#include "SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

SchematicView::SchematicView(SchematicPanel *panel, Schematic *schematic)
    : QGraphicsView(new SchematicCanvas(panel, schematic)), schematic(schematic) {
    scene()->setParent(this);
    setAcceptDrops(true);

    connect(schematic, &Schematic::panChanged,
            this, &SchematicView::pan);
    connect(schematic, &Schematic::zoomChanged,
            this, &SchematicView::zoom);

    // set properties
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);

    setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);
    pan(schematic->pan());

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void SchematicView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning = true;
        startMousePos = event->pos();
        startPan = schematic->pan();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }

    QGraphicsView::mousePressEvent(event);
}

void SchematicView::mouseMoveEvent(QMouseEvent *event) {
    if (isPanning) {
        auto mouseDelta = event->pos() - startMousePos;
        schematic->setPan(startPan - mouseDelta / lastScale);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void SchematicView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning = false;
        QApplication::restoreOverrideCursor();
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void SchematicView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    pan(schematic->pan());
}

void SchematicView::wheelEvent(QWheelEvent *event) {
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
        auto lastScaledPan = translatedEventPos / zoomToScale(schematic->zoom());
        auto delta = event->delta() / 1200.f;
        schematic->setZoom(schematic->zoom() + delta);
        schematic->setPan(schematic->pan() - translatedEventPos / zoomToScale(schematic->zoom()) + lastScaledPan);
    }
}

void SchematicView::dragEnterEvent(QDragEnterEvent *event) {
    if (!event->mimeData()->hasFormat("application/axiom-partial-surface")) return;

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
    startMousePos = QPoint(scenePos.x(), scenePos.y());
}

void SchematicView::dragMoveEvent(QDragMoveEvent *event) {
    auto mouseDelta = mapToScene(event->pos()) - startMousePos;
    schematic->dragTo(QPoint(
        mouseDelta.x() / SchematicCanvas::nodeGridSize.width(),
        mouseDelta.y() / SchematicCanvas::nodeGridSize.height()
    ));
}

void SchematicView::dragLeaveEvent(QDragLeaveEvent *event) {
    schematic->finishDragging();
    schematic->project()->history.cancelAction(HistoryList::ActionType::PLACE_MODULE);
}

void SchematicView::dropEvent(QDropEvent *event) {
    schematic->finishDragging();
    schematic->project()->history.endAction(HistoryList::ActionType::PLACE_MODULE);
    setFocus(Qt::OtherFocusReason);
}

void SchematicView::pan(QPointF pan) {
    centerOn(pan);
}

void SchematicView::zoom(float zoom) {
    auto newScale = zoomToScale(zoom);
    auto scaleChange = newScale / lastScale;
    lastScale = newScale;
    scale(scaleChange, scaleChange);
}

float SchematicView::zoomToScale(float zoom) {
    return std::pow(20.f, zoom);
}
