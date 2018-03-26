#include "SchematicView.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsItem>
#include <QtGui/QResizeEvent>

#include "editor/model/node/CustomNode.h"
#include "editor/model/schematic/Schematic.h"
#include "SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

SchematicView::SchematicView(SchematicPanel *panel, Schematic *schematic)
    : QGraphicsView(new SchematicCanvas(panel, schematic)), schematic(schematic) {
    scene()->setParent(this);

    scene()->setSceneRect(0, 0, width() * 2, height() * 2);

    // set properties
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);

    setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);
    centerOn(0, 0);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void SchematicView::pan(QPointF delta) {
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    auto newCenter = QPoint(viewport()->rect().width() / 2 - delta.x(), viewport()->rect().height() / 2 - delta.y());
    centerOn(mapToScene(newCenter));
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}

void SchematicView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning = true;
        lastMousePos = event->pos();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }

    QGraphicsView::mousePressEvent(event);
}

void SchematicView::mouseMoveEvent(QMouseEvent *event) {
    if (isPanning) {
        pan(mapToScene(event->pos()) - mapToScene(lastMousePos));
        lastMousePos = event->pos();
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
