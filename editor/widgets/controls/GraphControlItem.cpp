#include "GraphControlItem.h"

#include <QtGui/QPainter>
#include <cmath>

#include "editor/model/objects/GraphControl.h"

using namespace AxiomGui;

GraphControlZoom::GraphControlZoom(AxiomModel::GraphControl *control) : control(control) {
    setAcceptHoverEvents(true);
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
    auto knobColor = QColor(80, 80, 80);
    painter->setPen(QPen(knobColor, 2));
    if (isHovering) {
        painter->setBrush(QBrush(knobColor));
    }
    painter->drawEllipse(QPointF(rect.left() + paddingAmount + remappedZoom, rect.center().y()), 3, 3);
}

void GraphControlZoom::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    isHovering = true;
    update();
}

void GraphControlZoom::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    isHovering = false;
    update();
}

GraphControlItem::GraphControlItem(AxiomModel::GraphControl *control, AxiomGui::NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _zoomer(control) {
    control->zoomChanged.connect(this, &ControlItem::triggerUpdate);
    _zoomer.setParentItem(this);

    control->sizeChanged.connect(this, &GraphControlItem::positionChildren);
    positionChildren();
}

QRectF GraphControlItem::useBoundingRect() const {
    return drawBoundingRect().marginsRemoved(QMarginsF(3, 3, 3, 3));
}

QPainterPath GraphControlItem::controlPath() const {
    QPainterPath path;
    path.addRect(useBoundingRect());
    return path;
}

void GraphControlItem::paintControl(QPainter *painter) {
    auto boundingRect = useBoundingRect();

    auto fontHeight = painter->fontMetrics().height();
    auto headerHeight = fontHeight * 2;
    auto footerHeight = _zoomer.boundingRect().bottom();
    auto bodyRect = boundingRect.marginsRemoved(QMarginsF(0, headerHeight, 0, footerHeight));
    auto headerRect = QRectF(boundingRect.topLeft(), bodyRect.topRight());
    auto footerRect = QRectF(bodyRect.bottomLeft(), boundingRect.bottomRight());
    painter->fillRect(bodyRect, QBrush(QColor::fromRgb(10, 10, 10, 200)));
    painter->fillRect(headerRect, QBrush(QColor::fromRgb(30, 30, 30, 200)));
    painter->fillRect(footerRect, QBrush(QColor::fromRgb(30, 30, 30, 200)));

    painter->setPen(QPen(QColor(100, 100, 100)));
    painter->setBrush(Qt::NoBrush);

    auto sidePadding = 10;
    auto sidePaddingMargin = QMarginsF(sidePadding, 0, sidePadding, 0);
    bodyRect = bodyRect.marginsRemoved(sidePaddingMargin);
    headerRect = headerRect.marginsRemoved(sidePaddingMargin);
    footerRect = footerRect.marginsRemoved(sidePaddingMargin);

    auto widthSeconds = powf(2, control->zoom() + 1);
    auto pixelsPerSecond = bodyRect.width() / widthSeconds;
    paintTicks(painter, headerRect, pixelsPerSecond);
}

void GraphControlItem::paintTicks(QPainter *painter, QRectF rect, float pixelsPerSecond) {
    // draw the seconds tick marks
    auto numSeconds = (int) ceilf(rect.width() / pixelsPerSecond);
    for (auto i = 0; i < numSeconds + 1; i++) {
        auto centerXPos = rect.x() + i * pixelsPerSecond;
        painter->drawText(QRectF(centerXPos - 10, rect.y(), 20, 20), Qt::AlignHCenter | Qt::AlignTop,
                          QString::number(i));
        painter->drawLine(QPointF(centerXPos - 0.5, rect.bottom() - 10.5),
                          QPointF(centerXPos - 0.5, rect.bottom() - 0.5));
    }

    // draw the half-second tick marks
    for (auto i = 0; i < numSeconds; i++) {
        auto centerXPos = rect.x() + (i + 0.5) * pixelsPerSecond;
        painter->drawLine(QPointF(centerXPos - 0.5, rect.bottom() - 5.5),
                          QPointF(centerXPos - 0.5, rect.bottom() - 0.5));
    }
}

void GraphControlItem::positionChildren() {
    auto zoomBounds = _zoomer.boundingRect();
    auto selfBounds = useBoundingRect();

    // position the zoom control at the bottom right of our bounding rect
    _zoomer.setPos(selfBounds.right() - zoomBounds.right(), selfBounds.bottom() - zoomBounds.bottom());
}
