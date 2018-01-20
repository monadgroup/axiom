#include "BasicControl.h"

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtCore/QStateMachine>
#include <QtCore/QSignalTransition>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <cmath>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>

#include "editor/model/node/Node.h"
#include "editor/model/control/NodeValueControl.h"
#include "../node/NodeItem.h"
#include "../schematic/SchematicCanvas.h"
#include "editor/util.h"

using namespace AxiomGui;
using namespace AxiomModel;

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

BasicControl::BasicControl(NodeValueControl *control, SchematicCanvas *canvas)
        : ControlItem(control, canvas), control(control) {
    setAcceptHoverEvents(true);

    connect(control, &NodeValueControl::valueChanged,
            this, &BasicControl::triggerUpdate);
    connect(&control->sink, &ConnectionSink::connectionAdded,
            this, &BasicControl::triggerUpdate);
    connect(&control->sink, &ConnectionSink::connectionRemoved,
            this, &BasicControl::triggerUpdate);
    connect(&control->sink, &ConnectionSink::activeChanged,
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

QPainterPath BasicControl::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();

    QPainterPath path;
    switch (mode()) {
        case BasicMode::PLUG:
            path.addEllipse(getPlugBounds());
            break;
        case BasicMode::KNOB:
            path.addEllipse(getKnobBounds());
            break;
        case BasicMode::SLIDER_H:
            path.addRect(getSliderBounds(false));
            break;
        case BasicMode::SLIDER_V:
            path.addRect(getSliderBounds(true));
            break;
    }
    return path;
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
    ControlItem::mousePressEvent(event);
    if (event->isAccepted() || event->button() != Qt::LeftButton) return;

    event->accept();

    isDragging = true;
    beforeDragVal = control->value();
    mouseStartPoint = event->pos();
}

void BasicControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseMoveEvent(event);
    if (event->isAccepted() || !isDragging) return;

    event->accept();

    auto mouseDelta = event->pos() - mouseStartPoint;

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
    ControlItem::mouseReleaseEvent(event);
    if (event->isAccepted()) return;

    event->accept();

    isDragging = false;
}

void BasicControl::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseDoubleClickEvent(event);
    control->sink.setActive(false);
    emit mouseLeave();
}

void BasicControl::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink.setActive(true);
    emit mouseEnter();
}

void BasicControl::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink.setActive(false);
    emit mouseLeave();
}

void BasicControl::wheelEvent(QGraphicsSceneWheelEvent *event) {
    control->setValue(control->value() + event->delta() / 1200.f);
}

void BasicControl::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    auto clipboard = QGuiApplication::clipboard();
    float pasteVal = 0;
    auto canPaste = AxiomUtil::strToFloat(clipboard->text(), pasteVal);

    QMenu menu;
    auto setValAction = menu.addAction(tr("&Set Value..."));
    auto copyValAction = menu.addAction(tr("&Copy Value"));
    auto pasteValAction = menu.addAction(tr("&Paste Value"));
    pasteValAction->setEnabled(canPaste);
    menu.addSeparator();
    auto zeroAction = menu.addAction(tr("Set to &0"));
    auto oneAction = menu.addAction(tr("Set to &1"));
    menu.addSeparator();
    auto moveAction = menu.addAction(tr("&Move"));
    auto clearAction = menu.addAction(tr("C&lear Connections"));

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == setValAction) {
        // todo: show window to set value
    } else if (selectedAction == copyValAction) {
        clipboard->setText(QString::number(control->value()));
    } else if (selectedAction == pasteValAction) {
        // note: use the value from before showing menu, since we know it's valid
        control->setValue(pasteVal);
    } else if (selectedAction == zeroAction) {
        control->setValue(0);
    } else if (selectedAction == oneAction) {
        control->setValue(1);
    } else if (selectedAction == moveAction) {
        control->select(true);
    } else if (selectedAction == clearAction) {
        control->sink.clearConnections();
    }
}

QRectF BasicControl::getPlugBounds() const {
    auto br = aspectBoundingRect();
    auto scaledMargin = 0.1f * br.width();
    return br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}

QRectF BasicControl::getKnobBounds() const {
    auto br = aspectBoundingRect();
    auto scaledMargin = 0.1f * br.width();
    return br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}

QRectF BasicControl::getSliderBounds(bool vertical) const {
    auto br = flip(boundingRect(), vertical);
    auto scaledMargin = 0.1f * br.height();
    auto barHeight = br.height() / 2;
    auto barY = br.center().y() - barHeight / 2;
    return flip(QRectF(QPointF(br.x() + scaledMargin,
                               barY),
                       QSizeF(br.width() - scaledMargin * 2,
                              barHeight)), vertical);
}

void BasicControl::paintPlug(QPainter *painter) {
    auto scaledBorder = 0.06f * aspectBoundingRect().width();
    auto externBr = getPlugBounds();

    auto scaledBorderMargin = scaledBorder / 2;
    auto ellipseBr = externBr.marginsRemoved(
            QMarginsF(scaledBorderMargin, scaledBorderMargin, scaledBorderMargin, scaledBorderMargin));

    auto baseColor = QColor(10, 10, 10);
    auto activeColor = QColor(20, 20, 20);
    auto connectedActiveColor = QColor(52, 152, 219);
    auto connectedColor = QColor(141, 141, 141);
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, m_hoverState);

    if (control->sink.connections().empty()) {
        painter->setPen(QPen(QColor(30, 30, 30), scaledBorder));
    } else {
        painter->setPen(QPen(AxiomUtil::mixColor(connectedColor, connectedActiveColor, m_hoverState), 1));
    }
    painter->setBrush(QBrush(currentColor));
    painter->drawEllipse(ellipseBr);
}

void BasicControl::paintKnob(QPainter *painter) {
    auto aspectWidth = aspectBoundingRect().width();
    auto scaledThickness = (0.06f + 0.04f * m_hoverState) * aspectWidth;
    auto outerBr = getKnobBounds();
    auto ringBr = outerBr.marginsRemoved(
            QMarginsF(scaledThickness / 2, scaledThickness / 2, scaledThickness / 2, scaledThickness / 2));

    auto startAngle = 240 * 16;
    auto completeAngle = -300 * 16;

    auto baseColor = QColor(141, 141, 141);
    auto activeColor = QColor(52, 152, 219);
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, m_hoverState);

    // draw background
    painter->setPen(Qt::NoPen);
    if (!control->sink.connections().empty()) {
        auto activeBorderThickness = 0.02 * aspectWidth;
        painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, control->sink.active())));
        painter->drawEllipse(outerBr.marginsAdded(
                QMarginsF(activeBorderThickness, activeBorderThickness, activeBorderThickness, activeBorderThickness)));
    }

    painter->setBrush(QBrush(QColor(30, 30, 30)));
    painter->drawEllipse(outerBr);

    // draw markers
    auto scaledMarkerThickness = 0.02f * aspectWidth;
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

void BasicControl::paintSlider(QPainter *painter, bool vertical) {
    auto br = flip(getSliderBounds(vertical), vertical);
    auto scaledThickness = (0.12f + 0.08f * m_hoverState) * br.height();

    auto baseColor = QColor(141, 141, 141);
    auto activeColor = QColor(52, 152, 219);
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, m_hoverState);

    // draw background
    painter->setPen(Qt::NoPen);
    if (!control->sink.connections().empty()) {
        auto activeBorderThickness = 0.04 * br.height();
        painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, control->sink.active())));
        painter->drawRect(flip(
                br.marginsAdded(QMarginsF(activeBorderThickness, activeBorderThickness, activeBorderThickness,
                                          activeBorderThickness)),
                vertical
        ));
    }

    painter->setBrush(QBrush(QColor(30, 30, 30)));
    painter->drawRect(flip(br, vertical));

    // draw markers
    const auto markerCount = 12;
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

        auto markerX = br.left() + br.width() * (vertical ? 1 - markerVal : markerVal);

        auto shiftAmt = 2.5;
        if (i % 2 == 0) shiftAmt = 2;
        if (i == 0 || i == markerCount || i == markerCount / 2) shiftAmt = 1.5;
        painter->drawLine(
                flip(QPointF(markerX, br.y() + 1), vertical),
                flip(QPointF(markerX, br.y() + br.height() / shiftAmt), vertical)
        );
    }

    auto currentX = br.left() + br.width() * (vertical ? 1 - control->value() : control->value());

    auto posY = br.y() + scaledThickness / 2;
    auto leftPos = QPointF(vertical ? br.right() : br.left(), posY);
    auto rightPos = QPointF(vertical ? br.left() : br.right(), posY);
    auto currentPos = QPointF(currentX, posY);

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
