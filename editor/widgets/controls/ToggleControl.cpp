#include "ToggleControl.h"

#include <QtCore/QStateMachine>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSignalTransition>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>
#include <cmath>
#include <QtWidgets/QMenu>

#include "editor/model/control/NodeValueControl.h"
#include "editor/model/connection/ConnectionWire.h"
#include "editor/util.h"

using namespace AxiomGui;
using namespace AxiomModel;

ToggleControl::ToggleControl(NodeValueControl *control, SchematicCanvas *canvas)
        : ControlItem(control, canvas), control(control) {
    setAcceptHoverEvents(true);

    connect(control, &NodeValueControl::valueChanged,
            this, &ToggleControl::triggerUpdate);
    connect(&control->sink, &ConnectionSink::connectionAdded,
            this, &ToggleControl::triggerUpdate);
    connect(&control->sink, &ConnectionSink::connectionRemoved,
            this, &ToggleControl::triggerUpdate);
    connect(&control->sink, &ConnectionSink::activeChanged,
            this, &ToggleControl::triggerUpdate);

    // todo: refactor into ControlItem?
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

void ToggleControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto br = getBounds();
    auto scaleFactor = std::hypot(br.right() - br.left(), br.bottom() - br.top());
    auto borderMargin = QMarginsF(0.02 * br.width(), 0.02 * br.height(), 0.02 * br.width(), 0.02 * br.height());

    auto baseColor = QColor(45, 45, 45);
    auto activeColor = QColor(60, 60, 60);
    auto connectedColor = QColor(141, 141, 141);
    auto connectedActiveColor = QColor(52, 152, 219);

    // draw background
    painter->setPen(Qt::NoPen);
    if (!control->sink.connections().empty()) {
        painter->setBrush(QBrush(AxiomUtil::mixColor(connectedColor, connectedActiveColor, control->sink.active())));
        painter->drawRect(br.marginsAdded(borderMargin));
    }

    painter->setPen(QPen(QColor(0, 0, 0), 0.02 * scaleFactor));
    painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, m_hoverState)));
    painter->drawRect(br.marginsRemoved(borderMargin));

    // draw light
    auto lightRadius = 0.05 * scaleFactor;
    auto lightPos = QPointF(br.left() + br.width() / 2, br.top() + br.height() / 4);

    painter->setPen(Qt::NoPen);
    if (control->value()) {
        auto glowRadius = lightRadius * 2;
        QRadialGradient gradient(lightPos, glowRadius);
        gradient.setColorAt(0, connectedActiveColor);
        gradient.setColorAt(1, QColor(connectedActiveColor.red(), connectedActiveColor.green(),
                                      connectedActiveColor.blue(), 0));
        painter->setBrush(gradient);
        painter->drawEllipse(lightPos, glowRadius, glowRadius);
    }

    auto valColor = control->value() ? connectedActiveColor : QColor(0, 0, 0);
    painter->setBrush(QBrush(valColor));
    painter->drawEllipse(lightPos, lightRadius, lightRadius);
}

QPainterPath ToggleControl::shape() const {
    QPainterPath path;
    path.addRect(getBounds());
    return path;
}

void ToggleControl::setHoverState(float newHoverState) {
    if (newHoverState != m_hoverState) {
        m_hoverState = newHoverState;
        update();
    }
}

void ToggleControl::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mousePressEvent(event);
    event->accept();
}

void ToggleControl::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseReleaseEvent(event);
    if (event->button() != Qt::LeftButton || event->isAccepted() || !isEditable()) return;

    event->accept();
    control->setValue(control->value() ? 0 : 1);
}

void ToggleControl::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseDoubleClickEvent(event);
    control->sink.setActive(false);
    control->setValue(control->value() ? 0 : 1);
    emit mouseLeave();
}

void ToggleControl::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink.setActive(true);
    emit mouseEnter();
}

void ToggleControl::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink.setActive(false);
    emit mouseLeave();
}

void ToggleControl::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    QMenu menu;
    auto toggleAction = menu.addAction(tr(control->value() ? "Turn O&ff" : "Turn O&n"));
    menu.addSeparator();
    auto moveAction = menu.addAction(tr("&Move"));

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == toggleAction) {
        control->setValue(control->value() ? 0 : 1);
    } else if (selectedAction == moveAction) {
        control->select(true);
    }
}

QRectF ToggleControl::getBounds() const {
    auto br = boundingRect();
    auto scaledMargin = 0.1f * br.width();
    return br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}
