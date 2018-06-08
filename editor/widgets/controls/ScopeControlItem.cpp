#include "ScopeControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMenu>
#include <QtGui/QPainter>

#include "editor/model/objects/ScopeControl.h"

using namespace AxiomGui;
using namespace AxiomModel;

ScopeControlItem::ScopeControlItem(AxiomModel::ScopeControl *control, AxiomGui::NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
}

void ScopeControlItem::paintControl(QPainter *painter) {
    painter->setBrush(QBrush(Qt::black));
    painter->drawRect(useBoundingRect());
}

QPainterPath ScopeControlItem::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();
    return controlPath();
}

QRectF ScopeControlItem::useBoundingRect() const {
    return drawBoundingRect().marginsRemoved(QMarginsF(5, 5, 5, 5));
}

QPainterPath ScopeControlItem::controlPath() const {
    QPainterPath path;
    path.addRect(useBoundingRect());
    return path;
}

void ScopeControlItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    QMenu menu;
    buildMenuStart(menu);
    menu.addSeparator();
    buildMenuEnd(menu);

    menu.exec(event->screenPos());
}
