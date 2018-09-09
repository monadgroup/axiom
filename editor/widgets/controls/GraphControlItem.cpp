#include "GraphControlItem.h"

#include <QtCore/QStringBuilder>
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>
#include <cmath>

#include "../CommonColors.h"
#include "../FloatingValueEditor.h"
#include "editor/compiler/interface/Runtime.h"
#include "editor/model/ModelRoot.h"
#include "editor/model/actions/AddGraphPointAction.h"
#include "editor/model/actions/DeleteGraphPointAction.h"
#include "editor/model/actions/MoveGraphPointAction.h"
#include "editor/model/actions/SetGraphTagAction.h"
#include "editor/model/actions/SetGraphTensionAction.h"
#include "editor/model/objects/GraphControl.h"

using namespace AxiomGui;

static constexpr int SCROLL_MULTIPLIER = 1000;

static constexpr float MIN_ZOOM = -2;
static constexpr float MAX_ZOOM = 3;

static float remapSecondsToPixels(float seconds, float secondsPerPixel, float scroll) {
    return (seconds - scroll) * secondsPerPixel;
}

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

static QPainterPath generateTagPolygon() {
    QPainterPath path;
    path.moveTo(0, 0);
    path.lineTo(12, 0);
    path.lineTo(12, 20);
    path.lineTo(7, 15);
    path.lineTo(0, 15);
    path.closeSubpath();

    return path;
}

QPainterPath tagPolygon = generateTagPolygon();

GraphControlTicks::GraphControlTicks(GraphControlItem *item) : item(item) {
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    item->control->zoomChanged.connect(this, &GraphControlTicks::triggerUpdate);
    item->control->scrollChanged.connect(this, &GraphControlTicks::triggerUpdate);
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
    auto secondsOffset = (int) item->control->scroll();

    // draw the seconds tick marks
    for (auto i = 0; i < numSeconds + 2; i++) {
        auto offsetIndex = secondsOffset + i;
        auto centerXPos = clippedRect.x() + (offsetIndex - item->control->scroll()) * pixelsPerSecond;
        painter->drawText(QRectF(centerXPos - 10, rect.y(), 20, 20), Qt::AlignHCenter | Qt::AlignTop,
                          QString::number(offsetIndex));
        painter->drawLine(QPointF(centerXPos - 0.5, clippedRect.bottom() - 10.5),
                          QPointF(centerXPos - 0.5, clippedRect.bottom() - 0.5));
    }

    // draw the half-second tick marks
    for (auto i = 0; i < numSeconds; i++) {
        auto offsetIndex = secondsOffset + i;
        auto centerXPos = clippedRect.x() + (offsetIndex + 0.5 - item->control->scroll()) * pixelsPerSecond;
        painter->drawLine(QPointF(centerXPos - 0.5, clippedRect.bottom() - 5.5),
                          QPointF(centerXPos - 0.5, clippedRect.bottom() - 0.5));
    }

    // draw "tags" for each curve with a state
    auto controlState = item->control->state();
    for (uint8_t i = 0; i < controlState->curveCount + 1; i++) {
        auto curveState = controlState->curveStates[i];
        if (curveState == 0) continue;

        auto posSeconds = 0.f;
        if (i > 0) {
            posSeconds = controlState->curveEndPositions[i - 1];
        }

        auto posX = clippedRect.x() + (posSeconds - item->control->scroll()) * pixelsPerSecond;

        painter->setPen(QPen(QColor(20, 20, 20)));
        painter->setBrush(QBrush(QColor(80, 80, 80)));
        painter->drawPolygon(tagPolygon.toFillPolygon(QMatrix().translate(posX - 12, rect.bottom() - 20)));

        if (curveState == controlState->currentState + 1) {
            painter->setPen(QPen(QColor(253, 216, 53)));
        } else {
            painter->setPen(QPen(QColor(200, 200, 200)));
        }
        painter->setBrush(Qt::NoBrush);
        painter->drawText(QRectF(posX - 12, rect.bottom() - 20, 12, 15), Qt::AlignHCenter | Qt::AlignVCenter,
                          QString::number(curveState - 1));
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
    setCursor(QCursor(Qt::ArrowCursor));
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
    event->accept();

    isDragging = true;
    dragStartMousePos = event->scenePos();
    dragStartYVal = item->control->state()->curveStartVals[index];
    dragStartTime = index == 0 ? 0 : item->control->state()->curveEndPositions[index - 1];
    update();
}

void GraphControlPointKnob::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    item->setShowSnapMarks(true);
    auto controlState = item->control->state();

    auto mouseDelta = event->scenePos() - dragStartMousePos;
    auto yScale = maxY - minY;
    controlState->curveStartVals[index] = std::clamp(dragStartYVal - (float) (mouseDelta.y() / yScale), 0.f, 1.f);

    if (index != 0) {
        auto timeDelta = mouseDelta.x() / scale;
        controlState->curveEndPositions[index - 1] = std::clamp(
            (float) (round((dragStartTime + timeDelta) / snapSeconds) * snapSeconds), minSeconds, maxSeconds);
    }
}

void GraphControlPointKnob::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    isDragging = false;
    item->setShowSnapMarks(false);

    auto controlState = item->control->state();
    auto dragEndTime = index > 0 ? controlState->curveEndPositions[index - 1] : 0;
    auto dragEndYVal = controlState->curveStartVals[index];
    if (dragEndTime != dragStartTime || dragEndYVal != dragStartYVal) {
        item->control->root()->history().append(
            AxiomModel::MoveGraphPointAction::create(item->control->uuid(), index, dragStartTime, dragStartYVal,
                                                     dragEndTime, dragEndYVal, item->control->root()));
    }

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

void GraphControlPointKnob::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (index != 0) {
        auto controlState = item->control->state();
        auto pointTime = controlState->curveEndPositions[index - 1];
        auto pointValue = controlState->curveStartVals[index];
        auto pointTension = controlState->curveTension[index - 1];
        auto pointState = controlState->curveStates[index];
        item->control->root()->history().append(AxiomModel::DeleteGraphPointAction::create(
            item->control->uuid(), index, pointTime, pointValue, pointTension, pointState, item->control->root()));
        isDragging = false;
    }
}

void GraphControlPointKnob::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    uint8_t suggestedTag = 0;
    for (auto i = index - 1; i >= 0; i--) {
        auto curveState = item->control->state()->curveStates[i];
        if (curveState != 0) {
            suggestedTag = curveState;
            break;
        }
    }

    auto currentTag = item->control->state()->curveStates[index];
    if (suggestedTag + 1 == currentTag) {
        suggestedTag++;
    }

    auto clipboard = QGuiApplication::clipboard();
    bool validClipboardNumber;
    auto clipboardNumber = clipboard->text().toFloat(&validClipboardNumber);

    QMenu menu;

    auto setValueAction = menu.addAction("&Set Value...");
    auto copyValueAction = menu.addAction("&Copy Value");
    auto pasteValueAction = menu.addAction("&Paste Value");
    menu.addSeparator();
    auto clearTagAction = menu.addAction("&Clear Tag");
    auto tagSuggestedAction = menu.addAction("Tag &" % QString::number(suggestedTag));
    auto setTagAction = menu.addAction("&Set Tag...");
    menu.addSeparator();
    auto deleteAction = menu.addAction("&Delete");

    pasteValueAction->setEnabled(validClipboardNumber);
    clearTagAction->setEnabled(currentTag != 0);
    deleteAction->setEnabled(index != 0);

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == setValueAction) {
        auto editor =
            new FloatingValueEditor(QString::number(item->control->state()->curveStartVals[index]), event->scenePos());
        scene()->addItem(editor);
        connect(editor, &FloatingValueEditor::valueSubmitted, this, [this](QString newValue) {
            bool validInput;
            auto inputFloat = newValue.toFloat(&validInput);
            if (!validInput) return;

            auto controlState = item->control->state();
            auto oldVal = controlState->curveStartVals[index];
            auto newVal = std::clamp(inputFloat, 0.f, 1.f);
            if (newVal != oldVal) {
                auto pointTime = index > 0 ? controlState->curveEndPositions[index - 1] : 0;
                item->control->root()->history().append(AxiomModel::MoveGraphPointAction::create(
                    item->control->uuid(), index, pointTime, oldVal, pointTime, newVal, item->control->root()));
            }
        });
    } else if (selectedAction == copyValueAction) {
        clipboard->setText(QString::number(item->control->state()->curveStartVals[index], 'f', 2));
    } else if (validClipboardNumber && selectedAction == pasteValueAction) {
        auto controlState = item->control->state();
        auto oldVal = controlState->curveStartVals[index];
        auto newVal = std::clamp(clipboardNumber, 0.f, 1.f);
        if (newVal != oldVal) {
            auto pointTime = index > 0 ? controlState->curveEndPositions[index - 1] : 0;
            item->control->root()->history().append(AxiomModel::MoveGraphPointAction::create(
                item->control->uuid(), index, pointTime, oldVal, pointTime, newVal, item->control->root()));
        }
    } else if (selectedAction == clearTagAction) {
        auto oldState = item->control->state()->curveStates[index];
        uint8_t newState = 0;
        if (newState != oldState) {
            item->control->root()->history().append(AxiomModel::SetGraphTagAction::create(
                item->control->uuid(), index, oldState, newState, item->control->root()));
        }
    } else if (selectedAction == tagSuggestedAction) {
        auto oldState = item->control->state()->curveStates[index];
        auto newState = (uint8_t)(suggestedTag + 1);
        if (newState != oldState) {
            item->control->root()->history().append(AxiomModel::SetGraphTagAction::create(
                item->control->uuid(), index, oldState, newState, item->control->root()));
        }
    } else if (selectedAction == setTagAction) {
        QString defaultText = "";
        if (currentTag > 0) defaultText = QString::number(currentTag - 1);
        auto editor = new FloatingValueEditor(defaultText, event->scenePos());
        scene()->addItem(editor);
        connect(editor, &FloatingValueEditor::valueSubmitted, this, [this](QString newValue) {
            bool validInput;
            auto inputByte = newValue.toUInt(&validInput);
            if (!validInput || inputByte > UINT8_MAX - 1) return;

            auto oldState = item->control->state()->curveStates[index];
            auto newState = (uint8_t)(inputByte + 1);
            if (newState != oldState) {
                item->control->root()->history().append(AxiomModel::SetGraphTagAction::create(
                    item->control->uuid(), index, oldState, newState, item->control->root()));
            }
        });
    } else if (index != 0 && selectedAction == deleteAction) {
        auto controlState = item->control->state();
        auto pointTime = controlState->curveEndPositions[index - 1];
        auto pointValue = controlState->curveStartVals[index];
        auto pointTension = controlState->curveTension[index - 1];
        auto pointState = controlState->curveStates[index];
        item->control->root()->history().append(AxiomModel::DeleteGraphPointAction::create(
            item->control->uuid(), index, pointTime, pointValue, pointTension, pointState, item->control->root()));
    }
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

    auto controlState = control->state();
    auto dragEndTension = controlState->curveTension[index];
    if (dragEndTension != dragStartTension) {
        control->root()->history().append(AxiomModel::SetGraphTensionAction::create(
            control->uuid(), index, (float) dragStartTension, dragEndTension, control->root()));
    }

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

void GraphControlTensionKnob::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    auto clipboard = QGuiApplication::clipboard();
    bool validClipboardNumber;
    auto clipboardNumber = clipboard->text().toFloat(&validClipboardNumber);

    QMenu menu;
    auto setTensionAction = menu.addAction("&Set Tension...");
    auto copyTensionAction = menu.addAction("&Copy Tension");
    auto pasteTensionAction = menu.addAction("&Paste Tension");

    pasteTensionAction->setEnabled(validClipboardNumber);

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == setTensionAction) {
        auto editor = new FloatingValueEditor(QString::number(roundf(control->state()->curveTension[index] * 100)),
                                              event->scenePos());
        scene()->addItem(editor);
        connect(editor, &FloatingValueEditor::valueSubmitted, this, [this](QString newValue) {
            bool couldConvert;
            auto convertedVal = newValue.toFloat(&couldConvert);

            if (!couldConvert) return;
            auto oldTension = control->state()->curveTension[index];
            auto newTension = std::clamp(convertedVal / 100, -1.f, 1.f);
            ;
            if (oldTension != newTension) {
                control->root()->history().append(AxiomModel::SetGraphTensionAction::create(
                    control->uuid(), index, oldTension, newTension, control->root()));
            }
        });
    } else if (selectedAction == copyTensionAction) {
        clipboard->setText(QString::number(roundf(control->state()->curveTension[index] * 100)));
    } else if (validClipboardNumber && selectedAction == pasteTensionAction) {
        auto oldTension = control->state()->curveTension[index];
        auto newTension = std::clamp(clipboardNumber / 100, -1.f, 1.f);
        ;
        if (oldTension != newTension) {
            control->root()->history().append(AxiomModel::SetGraphTensionAction::create(
                control->uuid(), index, oldTension, newTension, control->root()));
        }
    }
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

    // create/remove QGraphicsPathItems so we have the correct amount
    while (_curves.size() < state->curveCount) {
        _curves.push_back(std::make_unique<QGraphicsPathItem>());
        auto &curve = _curves[_curves.size() - 1];
        curve->setParentItem(this);
        curve->setZValue(0);
    }
    while (_tensionKnobs.size() < state->curveCount) {
        _tensionKnobs.push_back(std::make_unique<GraphControlTensionKnob>(item->control, _tensionKnobs.size()));
        auto &tensionKnob = _tensionKnobs[_tensionKnobs.size() - 1];
        tensionKnob->setParentItem(this);
        tensionKnob->setZValue(1);
    }
    while (_pointKnobs.size() < state->curveCount + 1u) {
        _pointKnobs.push_back(std::make_unique<GraphControlPointKnob>(item, _pointKnobs.size()));
        auto &pointKnob = _pointKnobs[_pointKnobs.size() - 1];
        pointKnob->setParentItem(this);
        pointKnob->setZValue(2);
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
    auto scrollSeconds = item->control->scroll() * pixelsPerSecond;
    for (uint8_t curveIndex = 0; curveIndex < state->curveCount; curveIndex++) {
        auto startSeconds = 0.f;
        if (curveIndex > 0) {
            startSeconds = state->curveEndPositions[curveIndex - 1];
        }

        auto endSeconds = state->curveEndPositions[curveIndex];
        auto startVal = state->curveStartVals[curveIndex];
        auto endVal = state->curveStartVals[curveIndex + 1];
        auto tension = state->curveTension[curveIndex];

        auto graphLeftPixels = drawBounds.x() + startSeconds * pixelsPerSecond - scrollSeconds;
        auto graphRightPixels = drawBounds.x() + endSeconds * pixelsPerSecond - scrollSeconds;
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
    firstPointKnob->setPos(drawBounds.x() - scrollSeconds,
                           drawBounds.bottom() - drawBounds.height() * state->curveStartVals[0]);
}

void GraphControlArea::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    event->accept();
}

void GraphControlArea::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    // if the pos is outside draw bounds, ignore
    if (!drawBounds.contains(event->pos())) {
        return;
    }

    auto widthSeconds = getWidthSeconds(item->control->zoom());
    auto pixelsPerSecond = drawBounds.width() / widthSeconds;
    auto snapSeconds = getSnapSeconds(widthSeconds, drawBounds.width());

    auto drawBounded = event->pos() - drawBounds.topLeft();
    auto newPointVal = 1 - drawBounded.y() / drawBounds.height();
    auto newPointTime =
        round((drawBounded.x() / pixelsPerSecond + item->control->scroll()) / snapSeconds) * snapSeconds;

    auto insertIndex = item->control->determineInsertIndex((float) newPointTime);
    if (insertIndex) {
        item->control->root()->history().append(AxiomModel::AddGraphPointAction::create(
            item->control->uuid(), *insertIndex, (float) newPointTime, (float) newPointVal, item->control->root()));
    }
}

ScrollBarGraphicsItem::ScrollBarGraphicsItem(Qt::Orientation orientation) : scrollBar(new QScrollBar(orientation)) {
    setAcceptHoverEvents(true);

    setWidget(scrollBar);

    connect(scrollBar, &QScrollBar::valueChanged, this, &ScrollBarGraphicsItem::triggerUpdate);
}

void ScrollBarGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    update();
    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void ScrollBarGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    update();
    QGraphicsProxyWidget::hoverLeaveEvent(event);
}

void ScrollBarGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    update();
    QGraphicsProxyWidget::hoverMoveEvent(event);
}

void ScrollBarGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    update();
    QGraphicsProxyWidget::mousePressEvent(event);
}

void ScrollBarGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    update();
    QGraphicsProxyWidget::mouseReleaseEvent(event);
}

void ScrollBarGraphicsItem::triggerUpdate() {
    update();
}

GraphControlItem::GraphControlItem(AxiomModel::GraphControl *control, AxiomGui::NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _ticks(this), _zoomer(control), _area(this),
      _scrollBar(Qt::Horizontal) {
    _zoomer.setParentItem(this);
    _ticks.setParentItem(this);
    _area.setParentItem(this);

    _scrollBar.setParentItem(this);
    connect(_scrollBar.scrollBar, &QScrollBar::valueChanged, this, &GraphControlItem::scrollBarChanged);

    control->zoomChanged.connect(this, &GraphControlItem::stateChange);
    control->scrollChanged.connect(this, &GraphControlItem::stateChange);
    control->stateChanged.connect(this, &GraphControlItem::stateChange);
    control->timeChanged.connect(this, &GraphControlItem::triggerUpdate);
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

    painter->setClipRect(bodyRect);

    if (_showSnapMarks) {
        painter->setPen(QPen(QColor(40, 40, 40)));
        auto snapSeconds = getSnapSeconds(widthSeconds, clippedBodyRect.width());
        auto snapDistancePixels = snapSeconds * pixelsPerSecond;
        auto snapLineCount = (int) ceil(clippedBodyRect.width() / snapDistancePixels);
        for (int i = 0; i < snapLineCount + 1; i++) {
            auto pos = clippedBodyRect.left() +
                       floor(i * snapDistancePixels - fmod(control->scroll(), snapSeconds) * pixelsPerSecond) - 0.5;
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

    auto state = control->state();

    // draw a line at the end of the last curve
    auto endPos = state->curveEndPositions[state->curveCount - 1];
    auto endPixels =
        floor(clippedBodyRect.left() + remapSecondsToPixels(endPos, (float) pixelsPerSecond, control->scroll()));
    if (endPixels > clippedBodyRect.left() && endPixels < clippedBodyRect.right()) {
        painter->drawLine(QPointF(endPixels - 0.5, bodyRect.top() - 0.5),
                          QPointF(endPixels - 0.5, bodyRect.bottom() - 0.5));
    }

    // draw lines for each tagged curve
    for (uint8_t i = 0; i < state->curveCount + 1; i++) {
        if (state->curveStates[i] == 0) continue;

        auto tagPosition = 0.f;
        if (state->curveCount > 0) {
            tagPosition = state->curveEndPositions[i - 1];
        }

        auto linePixels = floor(clippedBodyRect.left() +
                                remapSecondsToPixels(tagPosition, (float) pixelsPerSecond, control->scroll()));
        painter->drawLine(QPointF(linePixels - 0.5, bodyRect.top() - 0.5),
                          QPointF(linePixels - 0.5, bodyRect.bottom() - 0.5));
    }

    // convert the controls internal time in samples to beats to display
    auto currentRuntime = control->root()->runtime();
    if (currentRuntime) {
        auto timeBarPixels =
            clippedBodyRect.left() + remapSecondsToPixels((state->currentTimeSamples * currentRuntime->getBpm()) /
                                                              (currentRuntime->getSampleRate() * 60),
                                                          (float) pixelsPerSecond, control->scroll());
        painter->setPen(QPen(QColor(67, 160, 71)));
        painter->drawLine(QPointF(timeBarPixels - 0.5, bodyRect.top() - 0.5),
                          QPointF(timeBarPixels - 0.5, bodyRect.bottom() - 0.5));
    }
}

void GraphControlItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    QMenu menu;
    buildMenuStart(menu);
    menu.addSeparator();
    buildMenuEnd(menu);

    menu.exec(event->screenPos());
}

void GraphControlItem::scrollBarChanged(int newVal) {
    control->setScroll(newVal / (float) SCROLL_MULTIPLIER);
}

void GraphControlItem::positionChildren() {
    auto zoomBounds = _zoomer.boundingRect();
    auto selfBounds = useBoundingRect();

    _ticks.setPos(selfBounds.topLeft());

    // position the zoom control at the bottom right of our bounding rect
    _zoomer.setPos(selfBounds.right() - zoomBounds.right(), selfBounds.bottom() - zoomBounds.bottom());

    _scrollBar.setGeometry(QRectF(QPointF(selfBounds.left(), selfBounds.bottom() - zoomBounds.bottom()),
                                  QSizeF(selfBounds.width() - zoomBounds.width(), zoomBounds.height())));

    auto bodyRect =
        selfBounds.marginsRemoved(QMarginsF(0, _ticks.boundingRect().bottom(), 0, _zoomer.boundingRect().bottom()));
    auto sidePadding = 10;
    auto sidePaddingMargin = QMarginsF(sidePadding, sidePadding, sidePadding, sidePadding);
    auto clippedBodyRect = bodyRect.marginsRemoved(sidePaddingMargin);

    _area.updateBounds(bodyRect, clippedBodyRect);
}

void GraphControlItem::stateChange() {
    auto intMultiplier = SCROLL_MULTIPLIER;
    auto visibleSeconds = getWidthSeconds(control->zoom());
    _scrollBar.scrollBar->setPageStep((int) (visibleSeconds * intMultiplier));
    _scrollBar.scrollBar->setMaximum((int) ((getWidthSeconds(MAX_ZOOM) - visibleSeconds) * intMultiplier));

    _area.updateCurves();
    update();
}
