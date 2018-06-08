#include "ExtractControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMenu>

#include "editor/model/Project.h"

using namespace AxiomGui;
using namespace AxiomModel;

ExtractControlItem::ExtractControlItem(ExtractControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
    control->activeSlotsChanged.connect(this, &ExtractControlItem::triggerUpdate);
    control->connections().itemAdded.connect(this, &ExtractControlItem::triggerUpdate);
    control->connections().itemRemoved.connect(this, &ExtractControlItem::triggerUpdate);
}

void ExtractControlItem::paintControl(QPainter *painter) {
    extractPainter.paint(painter, aspectBoundingRect(), hoverState(), control->activeSlots(), outlineNormalColor(),
                         outlineActiveColor());
}

QPainterPath ExtractControlItem::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();
    return controlPath();
}

QRectF ExtractControlItem::useBoundingRect() const {
    return extractPainter.getBounds(aspectBoundingRect());
}

QPainterPath ExtractControlItem::controlPath() const {
    QPainterPath path;
    extractPainter.shape(path, drawBoundingRect());
    return path;
}

void ExtractControlItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    QMenu menu;
    buildMenuStart(menu);
    menu.addSeparator();
    buildMenuEnd(menu);

    menu.exec(event->screenPos());
}
