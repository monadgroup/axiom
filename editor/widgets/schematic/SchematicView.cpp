#include "SchematicView.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsItem>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsSceneWheelEvent>

#include "editor/model/node/CustomNode.h"
#include "editor/model/schematic/Schematic.h"
#include "SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

SchematicView::SchematicView(SchematicPanel *panel, Schematic *schematic)
    : QGraphicsView(new SchematicCanvas(panel, schematic)), schematic(schematic) {
    scene()->setParent(this);

    scene()->setSceneRect(0, 0, width() * 2, height() * 2);

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
