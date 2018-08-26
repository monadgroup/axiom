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

static float getWidthSeconds(float zoom) {
    return powf(2, zoom + 1);
}

static float tensionGraph(float x, float tension) {
    const float q = 20;

    if (tension >= 0) {
        return powf(x, powf(q, tension));
    } else {
        return 1 - powf(1 - x, powf(q, -tension));
    }
}

static void drawTensionGraph(QPainterPath &path, QPointF startLeft, QPointF endRight, float tension) {
    const int numLines = 10 + (int) (endRight.x() - startLeft.x()) / 10;

    path.moveTo(startLeft);
    for (int i = 1; i <= numLines; i++) {
        auto x = i / (float) numLines;
        auto mixAmt = tensionGraph(x, tension);
        path.lineTo(startLeft.x() + (endRight.x() - startLeft.x()) * x,
                    startLeft.y() + (endRight.y() - startLeft.y()) * mixAmt);
    }
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
    // item->moveHoverCursor(event->pos().x());
}

void GraphControlTicks::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    // item->showHoverCursor(event->pos().x());
}

void GraphControlTicks::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    // item->hideHoverCursor();
}

void GraphControlTicks::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // item->placePoint(event->pos().x());
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

const QPointF TENSION_KNOB_SIZE = QPointF(8, 8);

GraphControlTensionKnob::GraphControlTensionKnob(AxiomModel::GraphControl *control, uint8_t index)
    : control(control), index(index) {
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::SizeVerCursor));
}

QRectF GraphControlTensionKnob::boundingRect() const {
    return QRectF(-TENSION_KNOB_SIZE / 2, TENSION_KNOB_SIZE / 2);
}

void GraphControlTensionKnob::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(QPen(QColor(200, 200, 200)));
    if (isDragging) {
        painter->setBrush(QBrush(QColor(200, 200, 200, 150)));
    } else if (isHovering) {
        painter->setBrush(QBrush(QColor(200, 200, 200, 100)));
    } else {
        painter->setBrush(QBrush(QColor(200, 200, 200, 50)));
    }
    painter->drawEllipse(boundingRect());
}

void GraphControlTensionKnob::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = true;
    dragStartTension = control->state()->curveTension[index];
    dragStartMouseY = event->scenePos().y();
    update();
}

void GraphControlTensionKnob::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    auto deltaY = event->scenePos().y() - dragStartMouseY;
    auto newTension = std::clamp(dragStartTension + deltaY / movementRange * 2, -1., 1.);
    control->state()->curveTension[index] = (float) newTension;
}

void GraphControlTensionKnob::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    update();
}

void GraphControlTensionKnob::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    isHovering = true;
    update();
}

void GraphControlTensionKnob::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    isHovering = false;
    update();
}

GraphControlArea::GraphControlArea(AxiomModel::GraphControl *control) : control(control) {
    setFlag(QGraphicsPathItem::ItemClipsChildrenToShape, true);
}

QRectF GraphControlArea::boundingRect() const {
    return clipBounds;
}

void GraphControlArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {}

void GraphControlArea::updateBounds(QRectF newClipBounds, QRectF newDrawBounds) {
    clipBounds = newClipBounds;
    drawBounds = newDrawBounds;
    updateCurves();
}

void GraphControlArea::updateCurves() {
    auto state = control->state();
    if (!state) return;

    // create/remove QGraphicsPathItems so we have the correct amount
    while (_curves.size() < state->curveCount) {
        _curves.push_back(std::make_unique<QGraphicsPathItem>());
        _curves[_curves.size() - 1]->setParentItem(this);

        _tensionKnobs.push_back(std::make_unique<GraphControlTensionKnob>(control, _tensionKnobs.size()));
        _tensionKnobs[_tensionKnobs.size() - 1]->setParentItem(this);
    }
    while (_curves.size() > state->curveCount) {
        _curves.pop_back();
    }

    auto widthSeconds = getWidthSeconds(control->zoom());
    auto pixelsPerSecond = drawBounds.width() / widthSeconds;
    for (uint8_t curveIndex = 0; curveIndex < state->curveCount; curveIndex++) {
        auto startSeconds = 0.f;
        if (curveIndex > 0) {
            startSeconds = state->curveEndPositions[curveIndex - 1];
        }

        auto endSeconds = state->curveEndPositions[curveIndex];
        auto startVal = state->curveStartVals[curveIndex];
        auto endVal = state->curveStartVals[curveIndex + 1];
        auto tension = state->curveTension[curveIndex];

        auto graphLeftPixels = drawBounds.x() + startSeconds * pixelsPerSecond;
        auto graphRightPixels = drawBounds.x() + endSeconds * pixelsPerSecond;
        auto graphY1Pixels = drawBounds.bottom() - drawBounds.height() * startVal;
        auto graphY2Pixels = drawBounds.bottom() - drawBounds.height() * endVal;

        QPainterPath newPath;
        drawTensionGraph(newPath, QPointF(graphLeftPixels, graphY1Pixels), QPointF(graphRightPixels, graphY2Pixels),
                         tension);
        auto &curve = _curves[curveIndex];
        curve->setPen(QPen(CommonColors::numNormal, 2));
        curve->setPath(newPath);

        auto &tensionKnob = _tensionKnobs[curveIndex];
        auto tensionY = graphY1Pixels + (graphY2Pixels - graphY1Pixels) * tensionGraph(0.5f, tension);
        tensionKnob->setPos((graphLeftPixels + graphRightPixels) / 2, tensionY);
        tensionKnob->movementRange = graphY1Pixels - graphY2Pixels;
    }
}

GraphControlItem::GraphControlItem(AxiomModel::GraphControl *control, AxiomGui::NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _ticks(this), _zoomer(control), _area(control) {
    _zoomer.setParentItem(this);
    _ticks.setParentItem(this);
    _area.setParentItem(this);

    control->zoomChanged.connect(this, &GraphControlItem::stateChange);
    control->stateChanged.connect(this, &GraphControlItem::stateChange);
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

    auto headerHeight = _ticks.boundingRect().bottom();
    auto footerHeight = _zoomer.boundingRect().bottom();
    auto bodyRect = boundingRect.marginsRemoved(QMarginsF(0, headerHeight, 0, footerHeight));
    auto footerRect = QRectF(bodyRect.bottomLeft(), boundingRect.bottomRight());
    painter->fillRect(bodyRect, QBrush(QColor::fromRgb(10, 10, 10)));
    painter->fillRect(footerRect, QBrush(QColor::fromRgb(30, 30, 30)));

    auto sidePadding = 10;
    auto sidePaddingMargin = QMarginsF(sidePadding, sidePadding, sidePadding, sidePadding);
    auto clippedBodyRect = bodyRect.marginsRemoved(sidePaddingMargin);

    painter->setPen(QPen(QColor(70, 70, 70)));
    painter->drawLine(QPointF(bodyRect.left() - 0.5, clippedBodyRect.top() - 0.5),
                      QPointF(bodyRect.right() - 0.5, clippedBodyRect.top() - 0.5));
    painter->drawLine(QPointF(bodyRect.left() - 0.5, clippedBodyRect.bottom() - 0.5),
                      QPointF(bodyRect.right() - 0.5, clippedBodyRect.bottom() - 0.5));
    painter->drawLine(QPointF(clippedBodyRect.left() - 0.5, bodyRect.top() - 0.5),
                      QPointF(clippedBodyRect.left() - 0.5, bodyRect.bottom() - 0.5));
    painter->drawLine(QPointF(clippedBodyRect.right() - 0.5, bodyRect.top() - 0.5),
                      QPointF(clippedBodyRect.right() - 0.5, bodyRect.bottom() - 0.5));

    // draw a line at the end of the last curve
    auto state = control->state();
    if (!state) return;
    auto widthSeconds = getWidthSeconds(control->zoom());
    auto pixelsPerSecond = clippedBodyRect.width() / widthSeconds;
    auto endPos = state->curveEndPositions[state->curveCount - 1];
    auto endPixels = floor(clippedBodyRect.left() + endPos * pixelsPerSecond);
    if (endPixels < clippedBodyRect.right()) {
        painter->drawLine(QPointF(endPixels - 0.5, bodyRect.top() - 0.5),
                          QPointF(endPixels - 0.5, bodyRect.bottom() - 0.5));
    }
}

void GraphControlItem::positionChildren() {
    auto zoomBounds = _zoomer.boundingRect();
    auto selfBounds = useBoundingRect();

    _ticks.setPos(selfBounds.topLeft());

    // position the zoom control at the bottom right of our bounding rect
    _zoomer.setPos(selfBounds.right() - zoomBounds.right(), selfBounds.bottom() - zoomBounds.bottom());

    auto bodyRect =
        selfBounds.marginsRemoved(QMarginsF(0, _ticks.boundingRect().bottom(), 0, _zoomer.boundingRect().bottom()));
    auto sidePadding = 10;
    auto sidePaddingMargin = QMarginsF(sidePadding, sidePadding, sidePadding, sidePadding);
    auto clippedBodyRect = bodyRect.marginsRemoved(sidePaddingMargin);

    _area.updateBounds(QRectF(bodyRect.topLeft(), QPointF(clippedBodyRect.right() - 1., bodyRect.bottom())),
                       clippedBodyRect);
}

void GraphControlItem::stateChange() {
    _area.updateCurves();
    update();
}
