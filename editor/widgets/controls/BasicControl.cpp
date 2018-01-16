#include "BasicControl.h"

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <cmath>
#include <QtCore/QStateMachine>
#include <QtCore/QSignalTransition>
#include <QtCore/QPropertyAnimation>
#include <iostream>

#include "editor/model/control/NodeValueControl.h"
#include "../node/NodeItem.h"
#include "../schematic/SchematicCanvas.h"
#include "editor/util.h"

using namespace AxiomGui;
using namespace AxiomModel;

BasicControl::BasicControl(NodeValueControl *control, NodeItem *parent) : control(control), parent(parent) {
    setAcceptHoverEvents(true);

    connect(control, &NodeValueControl::beforeSizeChanged,
            this, &BasicControl::triggerGeometryChange);

    connect(control, &NodeValueControl::valueChanged,
            this, &BasicControl::triggerUpdate);

    connect(control, &NodeValueControl::selected,
            this, &BasicControl::triggerUpdate);
    connect(control, &NodeValueControl::deselected,
            this, &BasicControl::triggerUpdate);

    auto machine = new QStateMachine();
    auto unhoveredState = new QState(machine);
    unhoveredState->assignProperty(this, "hoverState", 0);
    machine->setInitialState(unhoveredState);

    auto hoveredState = new QState(machine);
    hoveredState->assignProperty(this, "hoverState", 1);

    auto mouseEnterTransition = unhoveredState->addTransition(this, SIGNAL(mouseEnter()), hoveredState);
    auto enterAnim = new QPropertyAnimation(this, "hoverState");
    enterAnim->setDuration(100);
    mouseEnterTransition->addAnimation(enterAnim);

    auto mouseLeaveTransition = hoveredState->addTransition(this, SIGNAL(mouseLeave()), unhoveredState);
    auto leaveAnim = new QPropertyAnimation(this, "hoverState");
    leaveAnim->setDuration(100);
    mouseLeaveTransition->addAnimation(leaveAnim);

    machine->start();
}

QRectF BasicControl::boundingRect() const {
    return {
            QPoint(0, 0),
            SchematicCanvas::controlRealSize(control->size())
    };
}

QRectF BasicControl::aspectBoundingRect() const {
    auto bound = boundingRect();
    if (bound.size().width() > bound.size().height()) {
        return {
                QPointF(
                        bound.topLeft().x() + bound.size().width() / 2 - bound.size().height() / 2,
                        bound.topLeft().y()
                ),
                QSizeF(
                        bound.size().height(),
                        bound.size().height()
                )
        };
    } else {
        return {
                QPointF(
                        bound.topLeft().x(),
                        bound.topLeft().y() + bound.size().height() / 2 - bound.size().width() / 2
                ),
                QSizeF(
                        bound.size().width(),
                        bound.size().width()
                )
        };
    }
}

void BasicControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    switch (mode()) {
        case BasicMode::PLUG:
            paintPlug(painter);
            break;
        case BasicMode::KNOB:
            paintKnob(painter);
            break;
        case BasicMode::SLIDER_H:
            paintSlider(painter, false);
            break;
        case BasicMode::SLIDER_V:
            paintSlider(painter, true);
            break;
    }
}

BasicControl::BasicMode BasicControl::mode() const {
    auto rect = boundingRect();

    if (rect.width() < 50 || rect.height() < 50) return BasicMode::PLUG;
    if (rect.width() >= rect.height() * 2) return BasicMode::SLIDER_H;
    if (rect.height() >= rect.width() * 2) return BasicMode::SLIDER_V;
    return BasicMode::KNOB;
}

void BasicControl::setHoverState(float newHoverState) {
    if (newHoverState != m_hoverState) {
        m_hoverState = newHoverState;
        update();
    }
}

void BasicControl::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;

    isDragging = true;
    beforeDragVal = control->value();
    dragMouseStart = event->pos();
}

void BasicControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->pos() - dragMouseStart;

    auto accuracyDelta = mouseDelta.x();
    auto motionDelta = mouseDelta.y();
    auto scaleFactor = boundingRect().height();

    if (mode() == BasicMode::SLIDER_H) {
        accuracyDelta = mouseDelta.y();
        motionDelta = -mouseDelta.x();
        scaleFactor = boundingRect().width();
    }

    auto accuracy = scaleFactor * 2 + (float) std::abs(accuracyDelta) * 100 / scaleFactor;
    control->setValue(beforeDragVal - (float) motionDelta / accuracy);
}

void BasicControl::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
}

void BasicControl::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    emit mouseEnter();
}

void BasicControl::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    emit mouseLeave();
}

void BasicControl::triggerGeometryChange() {
    prepareGeometryChange();
}

void BasicControl::triggerUpdate() {
    update();
}

void BasicControl::paintPlug(QPainter *painter) {
    auto br = aspectBoundingRect();
    auto scaledBorder = 0.06f * br.width();
    auto scaledMargin = 0.1f * br.width() + scaledBorder / 2;
    auto marginBr = br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));

    auto baseColor = QColor(10, 10, 10);
    auto activeColor = QColor(20, 20, 20);
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, m_hoverState);

    painter->setPen(QPen(QColor(30, 30, 30), scaledBorder));
    painter->setBrush(QBrush(currentColor));
    painter->drawEllipse(marginBr);
}

void BasicControl::paintKnob(QPainter *painter) {
    auto br = aspectBoundingRect();
    auto scaledMargin = 0.1f * br.width();
    auto scaledThickness = (0.06f + 0.04f * m_hoverState) * br.width();
    auto outerBr = br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
    auto ringBr = outerBr.marginsRemoved(
            QMarginsF(scaledThickness / 2, scaledThickness / 2, scaledThickness / 2, scaledThickness / 2));

    auto startAngle = 240 * 16;
    auto completeAngle = -300 * 16;

    auto baseColor = QColor(141, 141, 141);
    auto activeColor = QColor(52, 152, 219);
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, m_hoverState);

    // draw background
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(30, 30, 30)));
    painter->drawEllipse(outerBr);

    // draw markers
    auto scaledMarkerThickness = 0.02f * br.width();
    auto centerP = outerBr.center();

    const auto markerCount = 8;
    auto activeMarkerPen = QPen(QColor(0, 0, 0), scaledMarkerThickness);
    for (auto i = 0; i <= markerCount; i++) {
        auto markerVal = (float) i / markerCount;
        if (markerVal < control->value() || (markerVal == 1 && control->value() == 1)) {
            activeMarkerPen.setColor(currentColor);
            painter->setPen(activeMarkerPen);
        } else {
            activeMarkerPen.setColor(QColor(0, 0, 0));
            painter->setPen(activeMarkerPen);
        }

        auto markerAngle = startAngle / 2880. * M_PI + markerVal * completeAngle / 2880. * M_PI;
        auto markerP = centerP + QPointF(
                outerBr.width() / 2 * std::cos(markerAngle),
                -outerBr.height() / 2 * std::sin(markerAngle)
        );
        painter->drawLine((centerP + 2 * markerP) / 3, (centerP + 10 * markerP) / 11);
    }

    // draw background ring
    auto pen = QPen(QColor(0, 0, 0), scaledThickness);
    pen.setCapStyle(Qt::FlatCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawArc(ringBr, startAngle + completeAngle * control->value(),
                     completeAngle - completeAngle * control->value());

    // draw filled ring
    pen.setColor(currentColor);
    painter->setPen(pen);
    painter->drawArc(ringBr, startAngle, completeAngle * control->value());
}

static QPointF flip(QPointF a, bool yes) {
    if (yes) {
        return {a.y(), a.x()};
    }
    return a;
}

static QSizeF flip(QSizeF a, bool yes) {
    if (yes) {
        return {a.height(), a.width()};
    }
    return a;
}

static QRectF flip(QRectF a, bool yes) {
    if (yes) {
        return {flip(a.topLeft(), yes), flip(a.size(), yes)};
    }
    return a;
}

void BasicControl::paintSlider(QPainter *painter, bool vertical) {
    auto br = flip(boundingRect(), vertical);

    auto scaledMargin = 0.1f * br.height();
    auto scaledThickness = (0.06f + 0.04f * m_hoverState) * br.height();
    auto scaledControlSize = QSizeF(0.15f * br.height(), 0.15f * br.height());
    auto marginBr = br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));

    auto baseColor = QColor(141, 141, 141);
    auto activeColor = QColor(52, 152, 219);
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, m_hoverState);

    auto barLeft = marginBr.left() + scaledControlSize.width() / 2;
    auto barRight = marginBr.right() - scaledControlSize.width() / 2;
    auto barY = marginBr.center().y();

    // draw markers
    const auto markerCount = 10;
    auto activeMarkerPen = QPen(QColor(0, 0, 0), 1);
    for (auto i = 0; i <= markerCount; i++) {
        auto markerVal = (float) i / markerCount;
        if (markerVal < control->value() || (markerVal == 1 && control->value() == 1)) {
            activeMarkerPen.setColor(currentColor);
            painter->setPen(activeMarkerPen);
        } else {
            activeMarkerPen.setColor(QColor(0, 0, 0));
            painter->setPen(activeMarkerPen);
        }

        auto markerX = barLeft + (barRight - barLeft) * markerVal;
        if (vertical) markerX = br.width() - markerX;

        auto isImportantMarker = i == 0 || i == markerCount || i == markerCount / 2;
        auto shiftAmt = br.height() / (isImportantMarker ? 6 : 8);
        painter->drawLine(
                flip(QPointF(markerX, barY), vertical),
                flip(QPointF(markerX, barY + shiftAmt), vertical)
        );
    }

    auto currentX = barLeft + (barRight - barLeft) * control->value();

    auto leftPos = QPointF(barLeft, barY);
    auto rightPos = QPointF(barRight, barY);
    auto currentPos = QPointF(currentX, barY);

    if (vertical) {
        leftPos.setX(barRight);
        rightPos.setX(barLeft);
        currentPos.setX(br.width() - currentX);
    }


    // draw background bar
    auto pen = QPen(QColor(0, 0, 0), scaledThickness);
    pen.setCapStyle(Qt::FlatCap);
    painter->setPen(pen);
    painter->drawLine(
            flip(currentPos, vertical),
            flip(rightPos, vertical)
    );

    // draw filled bar
    pen.setColor(currentColor);
    painter->setPen(pen);
    painter->drawLine(
            flip(leftPos, vertical),
            flip(currentPos, vertical)
    );
}
