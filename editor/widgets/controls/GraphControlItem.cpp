#include "GraphControlItem.h"

#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <cmath>

#include "../CommonColors.h"
#include "editor/model/objects/GraphControl.h"

using namespace AxiomGui;

static constexpr float MIN_ZOOM = -2;
static constexpr float MAX_ZOOM = 3;

float getWidthSeconds(float zoom) {
    return powf(2, zoom + 1);
}

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
    auto widthSeconds = getWidthSeconds(item->control->zoom());
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

void GraphControlTicks::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    item->placePoint(event->pos().x());
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

    // the zoom is in the range {MIN_ZOOM,MAX_ZOOM}, we want {0, lineLength}
    auto remappedZoom = (control->zoom() - MIN_ZOOM) / (MAX_ZOOM - MIN_ZOOM) * lineLength;
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
    // remap from {0, lineLength} to {MIN_ZOOM, MAX_ZOOM}
    auto rect = boundingRect();
    auto lineLength = rect.width() - 20;
    auto remappedZoom = (MAX_ZOOM - MIN_ZOOM) * (mouseX - 10) / lineLength + MIN_ZOOM;
    control->setZoom(qMax(MIN_ZOOM, qMin((float) remappedZoom, MAX_ZOOM)));
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

void GraphControlItem::placePoint(qreal pos) {
    // can't place the point if there are too many curves already
    auto state = control->state();
    if (state->curveCount == AxiomModel::GRAPH_CONTROL_CURVE_COUNT) {
        return;
    }

    auto bounds = useBoundingRect();
    auto left = qMax(0., qMin(pos, bounds.width() - 10));

    auto widthSeconds = getWidthSeconds(control->zoom());
    auto pixelsPerSecond = bounds.width() / widthSeconds;

    auto leftSeconds = left / pixelsPerSecond;

    if (state->curveCount == 0 || leftSeconds > state->curveEndPositions[state->curveCount - 1]) {
        // if the time is after the last, adding is trivial
        state->curveStartVals[state->curveCount + 1] = 0;
        state->curveEndPositions[state->curveCount] = (float) leftSeconds;
        state->curveTension[state->curveCount] = 0;
        state->curveStates[state->curveCount] = -1;
        state->curveCount++;
        update();
    } else {
        // figure out after which curve the new point should be placed
        ssize_t placePoint = -1;
        for (size_t i = 0; i < state->curveCount; i++) {
            if (leftSeconds < state->curveEndPositions[i]) {
                placePoint = i;
                break;
            }
        }
        assert(placePoint != -1);

        // move old values after the place point
        auto moveItems = state->curveCount - placePoint;
        memmove(&state->curveStartVals[placePoint + 2], &state->curveStartVals[placePoint + 1],
                sizeof(state->curveStartVals[0]) * moveItems);
        memmove(&state->curveEndPositions[placePoint + 1], &state->curveEndPositions[placePoint],
                sizeof(state->curveEndPositions[0]) * moveItems);
        memmove(&state->curveTension[placePoint + 1], &state->curveTension[placePoint],
                sizeof(state->curveTension[0]) * moveItems);
        memmove(&state->curveStates[placePoint + 1], &state->curveStates[placePoint],
                sizeof(state->curveStates[0]) * moveItems);

        state->curveStartVals[placePoint + 1] = 0;
        state->curveEndPositions[placePoint] = (float) leftSeconds;
        state->curveTension[placePoint] = 0;
        state->curveStates[placePoint] = -1;

        state->curveCount++;
        update();
    }
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
    auto sidePaddingMargin = QMarginsF(sidePadding, sidePadding, sidePadding, sidePadding);
    auto clippedBodyRect = bodyRect.marginsRemoved(sidePaddingMargin);

    auto controlState = control->state();
    auto widthSeconds = getWidthSeconds(control->zoom());
    auto pixelsPerSecond = clippedBodyRect.width() / widthSeconds;

    auto paintClipRect = bodyRect;
    paintClipRect.setRight(clippedBodyRect.right());
    painter->setClipRect(paintClipRect);
    painter->setPen(QPen(QColor(50, 50, 50)));
    QPainterPath curves;
    for (uint8_t curveIndex = 0; curveIndex < controlState->curveCount; curveIndex++) {
        auto startSeconds = 0.f;
        if (curveIndex > 0) {
            startSeconds = controlState->curveEndPositions[curveIndex - 1];
        }

        // don't continue further if it's past the edge of what we're drawing
        if (startSeconds >= widthSeconds) break;

        auto endSeconds = controlState->curveEndPositions[curveIndex];
        auto startVal = controlState->curveStartVals[curveIndex];
        auto endVal = controlState->curveStartVals[curveIndex + 1];
        auto tension = controlState->curveTension[curveIndex];

        auto endX = clippedBodyRect.x() + endSeconds * pixelsPerSecond;
        auto lineEndX = floor(endX) - 0.5;
        painter->drawLine(QPointF(lineEndX, bodyRect.top()), QPointF(lineEndX, bodyRect.bottom()));

        drawTensionGraph(curves,
                         QPointF(clippedBodyRect.x() + startSeconds * pixelsPerSecond,
                                 clippedBodyRect.bottom() - clippedBodyRect.height() * startVal),
                         QPointF(endX, clippedBodyRect.bottom() - clippedBodyRect.height() * endVal), tension);
    }

    painter->setPen(QPen(QColor(70, 70, 70)));
    painter->drawLine(QPointF(bodyRect.left() - 0.5, clippedBodyRect.top() - 0.5),
                      QPointF(bodyRect.right() - 0.5, clippedBodyRect.top() - 0.5));
    painter->drawLine(QPointF(bodyRect.left() - 0.5, clippedBodyRect.bottom() - 0.5),
                      QPointF(bodyRect.right() - 0.5, clippedBodyRect.bottom() - 0.5));

    painter->setPen(QPen(CommonColors::numNormal, 2));
    painter->drawPath(curves);

    painter->setPen(QPen(QColor(70, 70, 70)));
    painter->drawLine(QPointF(clippedBodyRect.left() - 0.5, bodyRect.top() - 0.5),
                      QPointF(clippedBodyRect.left() - 0.5, bodyRect.bottom() - 0.5));
    painter->drawLine(QPointF(clippedBodyRect.right() - 0.5, bodyRect.top() - 0.5),
                      QPointF(clippedBodyRect.right() - 0.5, bodyRect.bottom() - 0.5));
}

void GraphControlItem::positionChildren() {
    auto zoomBounds = _zoomer.boundingRect();
    auto selfBounds = useBoundingRect();

    _ticks.setPos(selfBounds.topLeft());

    // position the zoom control at the bottom right of our bounding rect
    _zoomer.setPos(selfBounds.right() - zoomBounds.right(), selfBounds.bottom() - zoomBounds.bottom());
}
