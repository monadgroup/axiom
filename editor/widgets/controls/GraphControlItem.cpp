#include "GraphControlItem.h"

#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsView>
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
    const int numLines = 150 + (int) (endRight.x() - startLeft.x()) / 10;

    path.moveTo(startLeft);
    for (int i = 1; i <= numLines; i++) {
        auto x = i / (float) numLines;
        auto mixAmt = tensionGraph(x, tension);
        path.lineTo(startLeft.x() + (endRight.x() - startLeft.x()) * x,
                    startLeft.y() + (endRight.y() - startLeft.y()) * mixAmt);
    }
}

static double getSnapSeconds(double widthSeconds, double boxWidth) {
    const double snappingPoint = 2;
    return pow(snappingPoint, ceil(log(widthSeconds / boxWidth * 20) / log(snappingPoint)));
}

GraphControlTicks::GraphControlTicks(GraphControlItem *item) : item(item) {
    setAcceptHoverEvents(true);
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

const QPointF POINT_KNOB_SIZE = QPointF(5, 5);
const QPointF TENSION_KNOB_SIZE = QPointF(3, 3);

GraphControlPointKnob::GraphControlPointKnob(GraphControlItem *item, uint8_t index) : item(item), index(index) {
    setAcceptHoverEvents(true);
}

QRectF GraphControlPointKnob::boundingRect() const {
    return QRectF(-POINT_KNOB_SIZE, POINT_KNOB_SIZE);
}

void GraphControlPointKnob::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(QPen(CommonColors::numNormal));

    auto backColor = CommonColors::numNormal;
    if (isDragging) {
        backColor.setAlpha(255);
    } else if (isHovering) {
        backColor.setAlpha(150);
    } else {
        backColor.setAlpha(50);
    }
    painter->setBrush(backColor);
    painter->drawEllipse(boundingRect());
}

void GraphControlPointKnob::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = true;
    dragStartMousePos = event->scenePos();
    dragStartYVal = item->control->state()->curveStartVals[index];
    dragStartTime = index == 0 ? 0 : item->control->state()->curveEndPositions[index - 1];
    item->setShowSnapMarks(true);
    update();
}

void GraphControlPointKnob::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->scenePos() - dragStartMousePos;
    auto yScale = maxY - minY;
    item->control->state()->curveStartVals[index] =
        std::clamp(dragStartYVal - (float) (mouseDelta.y() / yScale), 0.f, 1.f);

    if (index != 0) {
        auto timeDelta = mouseDelta.x() / scale;
        item->control->state()->curveEndPositions[index - 1] = std::clamp(
            (float) (round((dragStartTime + timeDelta) / snapSeconds) * snapSeconds), minSeconds, maxSeconds);
    }
}

void GraphControlPointKnob::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    item->setShowSnapMarks(false);
    update();
}

void GraphControlPointKnob::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    isHovering = true;
    update();
}

void GraphControlPointKnob::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    isHovering = false;
    update();
}

GraphControlTensionKnob::GraphControlTensionKnob(AxiomModel::GraphControl *control, uint8_t index)
    : control(control), index(index) {
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::SizeVerCursor));
}

QRectF GraphControlTensionKnob::boundingRect() const {
    return QRectF(-TENSION_KNOB_SIZE, TENSION_KNOB_SIZE);
}

void GraphControlTensionKnob::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(QPen(CommonColors::numNormal));

    auto backColor = CommonColors::numNormal;
    if (isDragging) {
        backColor.setAlpha(255);
    } else if (isHovering) {
        backColor.setAlpha(150);
    } else {
        backColor.setAlpha(50);
    }
    painter->setBrush(backColor);
    painter->drawEllipse(boundingRect());
}

void GraphControlTensionKnob::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = true;
    QApplication::setOverrideCursor(Qt::BlankCursor);
    dragStartTension = control->state()->curveTension[index];
    dragStartMouseY = event->scenePos().y();
    update();
}

void GraphControlTensionKnob::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto deltaY = event->scenePos().y() - dragStartMouseY;
    auto newTension = std::clamp(dragStartTension + deltaY / movementRange, -1., 1.);
    control->state()->curveTension[index] = (float) newTension;
}

void GraphControlTensionKnob::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;

    auto view = scene()->views().first();
    auto scenePos = mapToScene(QPointF(0, 0));
    auto viewPos = view->mapFromScene(scenePos);
    QCursor::setPos(view->viewport()->mapToGlobal(viewPos));

    QApplication::restoreOverrideCursor();
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

GraphControlArea::GraphControlArea(GraphControlItem *item) : item(item) {
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
    auto state = item->control->state();
    if (!state) return;

    // create/remove QGraphicsPathItems so we have the correct amount
    while (_curves.size() < state->curveCount) {
        _curves.push_back(std::make_unique<QGraphicsPathItem>());
        _curves[_curves.size() - 1]->setParentItem(this);
    }
    while (_tensionKnobs.size() < state->curveCount) {
        _tensionKnobs.push_back(std::make_unique<GraphControlTensionKnob>(item->control, _tensionKnobs.size()));
        _tensionKnobs[_tensionKnobs.size() - 1]->setParentItem(this);
    }
    while (_pointKnobs.size() < state->curveCount + 1u) {
        _pointKnobs.push_back(std::make_unique<GraphControlPointKnob>(item, _pointKnobs.size()));
        _pointKnobs[_pointKnobs.size() - 1]->setParentItem(this);
    }

    while (_curves.size() > state->curveCount) {
        _curves.pop_back();
    }
    while (_tensionKnobs.size() > state->curveCount) {
        _tensionKnobs.pop_back();
    }
    while (_pointKnobs.size() > state->curveCount + 1u) {
        _pointKnobs.pop_back();
    }

    auto widthSeconds = getWidthSeconds(item->control->zoom());
    auto pixelsPerSecond = drawBounds.width() / widthSeconds;
    auto snapSeconds = getSnapSeconds(widthSeconds, drawBounds.width());
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

        auto &pointKnob = _pointKnobs[curveIndex + 1];
        pointKnob->setPos(graphRightPixels, graphY2Pixels);
        pointKnob->scale = pixelsPerSecond;
        pointKnob->snapSeconds = snapSeconds;
        pointKnob->minY = mapToScene(0, drawBounds.top()).y();
        pointKnob->maxY = mapToScene(0, drawBounds.bottom()).y();
        pointKnob->minSeconds = startSeconds;

        if (curveIndex < state->curveCount - 1) {
            pointKnob->maxSeconds = state->curveEndPositions[curveIndex + 1];
        } else {
            pointKnob->maxSeconds = getWidthSeconds(MAX_ZOOM);
        }
    }

    auto &firstPointKnob = _pointKnobs[0];
    firstPointKnob->minY = mapToScene(0, drawBounds.top()).y();
    firstPointKnob->maxY = mapToScene(0, drawBounds.bottom()).y();
    firstPointKnob->setPos(drawBounds.x(), drawBounds.bottom() - drawBounds.height() * state->curveStartVals[0]);
}

GraphControlItem::GraphControlItem(AxiomModel::GraphControl *control, AxiomGui::NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _ticks(this), _zoomer(control), _area(this) {
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

void GraphControlItem::setShowSnapMarks(bool value) {
    if (value != _showSnapMarks) {
        _showSnapMarks = value;
        update();
    }
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

    auto widthSeconds = getWidthSeconds(control->zoom());
    auto pixelsPerSecond = clippedBodyRect.width() / widthSeconds;

    if (_showSnapMarks) {
        painter->setPen(QPen(QColor(40, 40, 40)));
        auto snapDistancePixels = getSnapSeconds(widthSeconds, clippedBodyRect.width()) * pixelsPerSecond;
        auto snapLineCount = (int) ceil(clippedBodyRect.width() / snapDistancePixels);
        for (int i = 0; i < snapLineCount; i++) {
            auto pos = clippedBodyRect.left() + floor(i * snapDistancePixels) - 0.5;
            painter->drawLine(QPointF(pos, bodyRect.top() - 0.5), QPointF(pos, bodyRect.bottom() - 0.5));
        }
    }

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
