#include "MidiControl.h"

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtCore/QStateMachine>
#include <QtCore/QSignalTransition>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>
#include <QtWidgets/QMenu>

#include "editor/model/node/Node.h"
#include "editor/model/control/NodeMidiControl.h"
#include "editor/model/connection/ConnectionWire.h"
#include "editor/model/schematic/Schematic.h"
#include "editor/model/Project.h"
#include "../../util.h"

using namespace AxiomGui;
using namespace AxiomModel;

static std::vector<std::pair<QString, NodeMidiControl::Mode>> modes = {
    std::make_pair("&Plug", NodeMidiControl::Mode::PLUG),
    std::make_pair("&Piano", NodeMidiControl::Mode::PIANO)
};

MidiControl::MidiControl(AxiomModel::NodeMidiControl *control, SchematicCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
    setAcceptHoverEvents(true);

    connect(control, &NodeMidiControl::valueChanged,
            this, &MidiControl::triggerUpdate);
    connect(control, &NodeMidiControl::modeChanged,
            this, &MidiControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::connectionAdded,
            this, &MidiControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::connectionRemoved,
            this, &MidiControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::activeChanged,
            this, &MidiControl::triggerUpdate);

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

void MidiControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    ControlItem::paint(painter, option, widget);

    switch (control->mode()) {
        case NodeMidiControl::Mode::PLUG:
            plugPainter.paint(painter, aspectBoundingRect(), m_hoverState);
            break;
        case NodeMidiControl::Mode::PIANO:break;
    }
}

QPainterPath MidiControl::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();
    return controlPath();
}

void MidiControl::setHoverState(float newHoverState) {
    if (newHoverState != m_hoverState) {
        m_hoverState = newHoverState;
        update();
    }
}

QRectF MidiControl::useBoundingRect() const {
    switch (control->mode()) {
        case NodeMidiControl::Mode::PLUG:
            return plugPainter.getBounds(aspectBoundingRect());
        case NodeMidiControl::Mode::PIANO:break;
    }
    unreachable;
}

QPainterPath MidiControl::controlPath() const {
    QPainterPath path;
    switch (control->mode()) {
        case NodeMidiControl::Mode::PLUG:
            plugPainter.shape(path, aspectBoundingRect());
            break;
        case NodeMidiControl::Mode::PIANO:break;
    }
    return path;
}

void MidiControl::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mousePressEvent(event);
    event->accept();
}

void MidiControl::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseDoubleClickEvent(event);
    control->sink()->setActive(false);
    emit mouseLeave();
}

void MidiControl::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink()->setActive(true);
    emit mouseEnter();
}

void MidiControl::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink()->setActive(false);
    emit mouseLeave();
}

void MidiControl::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    QMenu menu;
    auto clearAction = menu.addAction(tr("C&lear Connections"));

    auto modeMenu = menu.addMenu(tr("&Display as..."));
    for (const auto &modePair : modes) {
        auto action = modeMenu->addAction(modePair.first);
        action->setCheckable(true);
        action->setChecked(control->mode() == modePair.second);

        connect(action, &QAction::triggered,
                [this, modePair]() {
                    control->setMode(modePair.second);
                });
    }

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
        control->node->parentSchematic->project()->build();
    } else if (selectedAction == moveAction) {
        control->select(true);
    } else if (selectedAction == nameShownAction) {
        control->setShowName(nameShownAction->isCheckable());
    } else if (exposedAction && selectedAction == exposedAction) {
        control->node->parentSchematic->exposeControl(control);
    }
}
