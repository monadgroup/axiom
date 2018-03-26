#include "ExtractControl.h"

#include <QtCore/QStateMachine>
#include <QtCore/QSignalTransition>
#include <QtCore/QPropertyAnimation>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMenu>
#include <editor/model/node/Node.h>
#include <cassert>

#include "editor/model/control/NodeExtractControl.h"
#include "editor/model/connection/ConnectionWire.h"
#include "editor/model/schematic/Schematic.h"

using namespace AxiomGui;
using namespace AxiomModel;

ExtractControl::ExtractControl(NodeExtractControl *control, SchematicCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
    setAcceptHoverEvents(true);

    connect(control, &NodeExtractControl::activeSlotsChanged,
            this, &ExtractControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::connectionAdded,
            this, &ExtractControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::connectionRemoved,
            this, &ExtractControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::activeChanged,
            this, &ExtractControl::triggerUpdate);

    auto machine = new QStateMachine(this);
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

void ExtractControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    ControlItem::paint(painter, option, widget);

    extractPainter.paint(painter, aspectBoundingRect(), m_hoverState, control->activeSlots(), outlineActiveColor());
}

QPainterPath ExtractControl::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();
    return controlPath();
}

void ExtractControl::setHoverState(float newHoverState) {
    if (newHoverState != m_hoverState) {
        m_hoverState = newHoverState;
        update();
    }
}

QRectF ExtractControl::useBoundingRect() const {
    return extractPainter.getBounds(aspectBoundingRect());
}

QPainterPath ExtractControl::controlPath() const {
    QPainterPath path;
    extractPainter.shape(path, drawBoundingRect());
    return path;
}

QColor ExtractControl::outlineNormalColor() const {
    switch (control->sink()->type) {
        case ConnectionSink::Type::NUMBER: return CommonColors::numWireNormal;
        case ConnectionSink::Type::MIDI: return CommonColors::midiWireNormal;
    }
}

QColor ExtractControl::outlineActiveColor() const {
    switch (control->sink()->type) {
        case ConnectionSink::Type::NUMBER: return CommonColors::numWireActive;
        case ConnectionSink::Type::MIDI: return CommonColors::midiWireActive;
    }
}

void ExtractControl::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mousePressEvent(event);
    event->accept();
    return;
}

void ExtractControl::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseDoubleClickEvent(event);
    control->sink()->setActive(false);
    emit mouseLeave();
}

void ExtractControl::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink()->setActive(true);
    emit mouseEnter();
}

void ExtractControl::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink()->setActive(false);
    emit mouseLeave();
}

void ExtractControl::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    QMenu menu;
    auto clearAction = menu.addAction(tr("C&lear Connections"));
    menu.addSeparator();
    auto moveAction = menu.addAction(tr("&Move"));
    auto nameShownAction = menu.addAction(tr("Show &Name"));
    nameShownAction->setCheckable(true);
    nameShownAction->setChecked(control->showName());

    QAction *exposedAction = nullptr;
    if (control->node->parentSchematic->canExposeControl()) {
        // todo: make this checkable
        exposedAction = menu.addAction(tr("&Expose"));
    }

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == clearAction) {
        control->sink()->clearConnections();
    } else if (selectedAction == moveAction) {
        control->select(true);
    } else if (selectedAction == nameShownAction) {
        control->setShowName(nameShownAction->isChecked());
    } else if (exposedAction && selectedAction == exposedAction) {
        control->node->parentSchematic->exposeControl(control);
    }
}
