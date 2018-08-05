#include "GraphControlItem.h"

#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <cmath>

#include "../CommonColors.h"
#include "editor/model/objects/GraphControl.h"

using namespace AxiomGui;

GraphControlTicks::GraphControlTicks(GraphControlItem *item) : item(item) {
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::IBeamCursor));
    item->control->zoomChanged.connect(this, &GraphControlTicks::triggerUpdate);
    item->control->beforeSizeChanged.connect(this, &GraphControlTicks::triggerGeometryChange);
}

void GraphControlTicks::triggerUpdate() {
    update();
}

void GraphControlTicks::triggerGeometryChange() {
    prepareGeometryChange();
}

QRectF GraphControlTicks::boundingRect() const {
    auto parentRect = item->useBoundingRect();
    return QRectF(QPointF(0, 0), QSizeF(parentRect.width(), 30));
}

void GraphControlTicks::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto rect = boundingRect();
    painter->fillRect(rect, QBrush(QColor::fromRgb(30, 30, 30)));

    painter->setPen(QPen(QColor(100, 100, 100)));
    painter->setBrush(Qt::NoBrush);

    auto clippedRect = rect.marginsRemoved(QMarginsF(10, 0, 10, 0));
    auto widthSeconds = powf(2, item->control->zoom() + 1);
    auto pixelsPerSecond = clippedRect.width() / widthSeconds;
    auto numSeconds = (int) ceilf(widthSeconds);

    painter->setClipRect(rect);

    // draw the seconds tick marks
    for (auto i = 0; i < numSeconds + 1; i++) {
        auto centerXPos = clippedRect.x() + i * pixelsPerSecond;
        painter->drawText(QRectF(centerXPos - 10, rect.y(), 20, 20), Qt::AlignHCenter | Qt::AlignTop,
                          QString::number(i));
        painter->drawLine(QPointF(centerXPos - 0.5, clippedRect.bottom() - 10.5),
                          QPointF(centerXPos - 0.5, clippedRect.bottom() - 0.5));
    }

    // draw the half-second tick marks
    for (auto i = 0; i < numSeconds; i++) {
        auto centerXPos = clippedRect.x() + (i + 0.5) * pixelsPerSecond;
        painter->drawLine(QPointF(centerXPos - 0.5, clippedRect.bottom() - 5.5),
                          QPointF(centerXPos - 0.5, clippedRect.bottom() - 0.5));
    }
}

void GraphControlTicks::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    item->moveHoverCursor(event->pos().x());
}

void GraphControlTicks::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    item->showHoverCursor(event->pos().x());
}

void GraphControlTicks::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    item->hideHoverCursor();
}

GraphControlZoom::GraphControlZoom(AxiomModel::GraphControl *control) : control(control) {
    setAcceptHoverEvents(true);
    control->zoomChanged.connect(this, &GraphControlZoom::triggerUpdate);
}

void GraphControlZoom::triggerUpdate() {
    update();
}

QRectF GraphControlZoom::boundingRect() const {
    return QRectF(0, 0, 100, 15);
}

void GraphControlZoom::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto rect = boundingRect();
    auto paddingAmount = 10;
    auto lineLength = rect.width() - paddingAmount * 2;

    painter->setPen(QPen(QColor(10, 10, 10), 3, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(rect.left() + paddingAmount, rect.center().y()),
                      QPointF(rect.right() - paddingAmount, rect.center().y()));

    // the zoom is in the range {-2,2}, we want {0, lineLength}
    auto remappedZoom = (control->zoom() + 2) / 4 * lineLength;
    QColor knobColor;
    if (isDragging) {
        knobColor = QColor(120, 120, 120);
    } else {
        knobColor = QColor(80, 80, 80);
    }

    painter->setPen(QPen(knobColor, 2));
    if (isHovering || isDragging) {
        painter->setBrush(QBrush(knobColor));
    }
    painter->drawEllipse(QPointF(rect.left() + paddingAmount + remappedZoom, rect.center().y()), 3, 3);
}

void GraphControlZoom::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = true;
    updateZoom(event->pos().x());
    update();
}

void GraphControlZoom::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    updateZoom(event->pos().x());
}

void GraphControlZoom::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    update();
}

void GraphControlZoom::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    isHovering = true;
    update();
    QGraphicsObject::hoverEnterEvent(event);
}

void GraphControlZoom::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    isHovering = false;
    update();
    QGraphicsObject::hoverLeaveEvent(event);
}

void GraphControlZoom::updateZoom(qreal mouseX) {
    // remap from {0, lineLength} to {-2, 2}
    auto rect = boundingRect();
    auto lineLength = rect.width() - 20;
    auto remappedZoom = 4 * (mouseX - 10) / lineLength - 2;
    control->setZoom(qMax(-2., qMin(remappedZoom, 2.)));
}

GraphControlItem::GraphControlItem(AxiomModel::GraphControl *control, AxiomGui::NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _ticks(this), _zoomer(control) {
    control->zoomChanged.connect(this, &ControlItem::triggerUpdate);
    _zoomer.setParentItem(this);
    _ticks.setParentItem(this);
    _hoverCursor.setParentItem(this);
    _hoverCursor.setPen(QPen(QColor(50, 50, 50)));
    _hoverCursor.setVisible(false);

    control->sizeChanged.connect(this, &GraphControlItem::positionChildren);
    positionChildren();
}

QRectF GraphControlItem::useBoundingRect() const {
    return drawBoundingRect().marginsRemoved(QMarginsF(3, 3, 3, 3));
}

void GraphControlItem::showHoverCursor(qreal pos) {
    moveHoverCursor(pos);
    _hoverCursor.setVisible(true);
}

void GraphControlItem::moveHoverCursor(qreal pos) {
    auto bounds = useBoundingRect();
    auto left = bounds.left() + qMax(10., qMin(pos, bounds.width() - 10)) - 0.5;
    _hoverCursor.setLine(left, bounds.top() + _ticks.boundingRect().height() - 0.5, left,
                         bounds.bottom() - _zoomer.boundingRect().height() - 0.5);
}

void GraphControlItem::hideHoverCursor() {
    _hoverCursor.setVisible(false);
}

QPainterPath GraphControlItem::controlPath() const {
    QPainterPath path;
    path.addRect(useBoundingRect());
    return path;
}

static float tensionGraph(float x, float tension) {
    const float q = 15;

    if (tension >= 0) {
        return powf(x, powf(q, tension));
    } else {
        return 1 - powf(1 - x, powf(q, -tension));
    }
}

void drawTensionGraph(QPainterPath &path, QPointF startLeft, QPointF endRight, float tension) {
    const int numLines = 10 + (int) (endRight.x() - startLeft.x()) / 10;

    path.moveTo(startLeft);
    for (int i = 1; i <= numLines; i++) {
        auto x = i / (float) numLines;
        auto mixAmt = tensionGraph(x, tension);
        path.lineTo(startLeft.x() + (endRight.x() - startLeft.x()) * x,
                    startLeft.y() + (endRight.y() - startLeft.y()) * mixAmt);
    }
}

void GraphControlItem::paintControl(QPainter *painter) {
    auto boundingRect = useBoundingRect();

    auto headerHeight = _ticks.boundingRect().bottom();
    auto footerHeight = _zoomer.boundingRect().bottom();
    auto bodyRect = boundingRect.marginsRemoved(QMarginsF(0, headerHeight, 0, footerHeight));
    auto footerRect = QRectF(bodyRect.bottomLeft(), boundingRect.bottomRight());
    painter->fillRect(bodyRect, QBrush(QColor::fromRgb(10, 10, 10)));
    painter->fillRect(footerRect, QBrush(QColor::fromRgb(30, 30, 30)));

    painter->setPen(QPen(QColor(100, 100, 100)));
    painter->setBrush(Qt::NoBrush);

    auto sidePadding = 10;
    auto sidePaddingMargin = QMarginsF(sidePadding, 0, sidePadding, 0);
    auto clippedBodyRect = bodyRect.marginsRemoved(sidePaddingMargin);

    painter->setPen(QPen(QColor(50, 50, 50)));
    painter->drawLine(clippedBodyRect.topLeft() - QPointF(0.5, 0.5), clippedBodyRect.bottomLeft() - QPointF(0.5, 0.5));
    painter->drawLine(clippedBodyRect.topRight() - QPointF(0.5, 0.5),
                      clippedBodyRect.bottomRight() - QPointF(0.5, 0.5));

    painter->setPen(QPen(CommonColors::numNormal, 2));

    QPainterPath tensionGraph;
    drawTensionGraph(tensionGraph, clippedBodyRect.bottomLeft(), clippedBodyRect.topRight(), hoverState() * 2 - 1);
    painter->drawPath(tensionGraph);
}

void GraphControlItem::positionChildren() {
    auto zoomBounds = _zoomer.boundingRect();
    auto selfBounds = useBoundingRect();

    _ticks.setPos(selfBounds.topLeft());

    // position the zoom control at the bottom right of our bounding rect
    _zoomer.setPos(selfBounds.right() - zoomBounds.right(), selfBounds.bottom() - zoomBounds.bottom());
}
