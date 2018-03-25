#include "NumControl.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtCore/QStateMachine>
#include <QtCore/QSignalTransition>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>

#include "editor/model/node/Node.h"
#include "editor/model/control/NodeNumControl.h"
#include "editor/model/schematic/Schematic.h"
#include "../node/NodeItem.h"
#include "../schematic/SchematicCanvas.h"
#include "editor/util.h"
#include "../FloatingValueEditor.h"
#include "../CommonColors.h"

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

static std::vector<std::pair<QString, NodeNumControl::Mode>> modes = {
    std::make_pair("&Plug", NodeNumControl::Mode::PLUG),
    std::make_pair("&Knob", NodeNumControl::Mode::KNOB),
    std::make_pair("&Horizontal Slider", NodeNumControl::Mode::SLIDER_H),
    std::make_pair("&Vertical Slider", NodeNumControl::Mode::SLIDER_V),
    std::make_pair("&Toggle Button", NodeNumControl::Mode::TOGGLE)
};

NumControl::NumControl(NodeNumControl *control, SchematicCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
    setAcceptHoverEvents(true);

    connect(control, &NodeNumControl::valueChanged,
            this, &NumControl::triggerUpdate);
    connect(control, &NodeNumControl::modeChanged,
            this, &NumControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::connectionAdded,
            this, &NumControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::connectionRemoved,
            this, &NumControl::triggerUpdate);
    connect(control->sink(), &ConnectionSink::activeChanged,
            this, &NumControl::triggerUpdate);

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

void NumControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    ControlItem::paint(painter, option, widget);

    switch (control->mode()) {
        case NodeNumControl::Mode::PLUG:
            plugPainter.paint(painter, aspectBoundingRect(), m_hoverState);
            break;
        case NodeNumControl::Mode::KNOB:
            knobPainter.paint(painter, aspectBoundingRect(), m_hoverState, getCVal());
            break;
        case NodeNumControl::Mode::SLIDER_H:
            sliderPainter.paint(painter, drawBoundingRect(), m_hoverState, getCVal(), false);
            break;
        case NodeNumControl::Mode::SLIDER_V:
            sliderPainter.paint(painter, drawBoundingRect(), m_hoverState, getCVal(), true);
            break;
        case NodeNumControl::Mode::TOGGLE:
            togglePainter.paint(painter, drawBoundingRect(), m_hoverState, getCVal());
            break;
    }
}

QPainterPath NumControl::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();
    return controlPath();
}

void NumControl::setHoverState(float newHoverState) {
    if (newHoverState != m_hoverState) {
        m_hoverState = newHoverState;
        update();
    }
}

QRectF NumControl::useBoundingRect() const {
    switch (control->mode()) {
        case NodeNumControl::Mode::PLUG:
            return plugPainter.getBounds(aspectBoundingRect());
        case NodeNumControl::Mode::KNOB:
            return knobPainter.getBounds(aspectBoundingRect());
        case NodeNumControl::Mode::SLIDER_H:
            return sliderPainter.getBounds(drawBoundingRect(), false);
        case NodeNumControl::Mode::SLIDER_V:
            return sliderPainter.getBounds(drawBoundingRect(), true);
        case NodeNumControl::Mode::TOGGLE:
            return togglePainter.getBounds(drawBoundingRect());
    }
}

QPainterPath NumControl::controlPath() const {
    QPainterPath path;
    switch (control->mode()) {
        case NodeNumControl::Mode::PLUG:
            plugPainter.shape(path, aspectBoundingRect());
            break;
        case NodeNumControl::Mode::KNOB:
            knobPainter.shape(path, aspectBoundingRect());
            break;
        case NodeNumControl::Mode::SLIDER_H:
            sliderPainter.shape(path, drawBoundingRect(), false);
            break;
        case NodeNumControl::Mode::SLIDER_V:
            sliderPainter.shape(path, drawBoundingRect(), true);
            break;
        case NodeNumControl::Mode::TOGGLE:
            togglePainter.shape(path, drawBoundingRect());
            break;
    }
    return path;
}

void NumControl::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mousePressEvent(event);
    if (event->isAccepted() || event->button() != Qt::LeftButton || control->mode() == NodeNumControl::Mode::PLUG) {
        return;
    }

    event->accept();

    if (control->mode() == NodeNumControl::Mode::TOGGLE) {
        auto cv = getCVal();
        auto isActive = cv.left != 0 || cv.right != 0;
        auto newVal = !isActive;
        setCVal(cv.withLR(newVal, newVal));
        return;
    }

    isDragging = true;
    beforeDragVal = getCVal();
    mouseStartPoint = event->pos();
}

void NumControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseMoveEvent(event);
    if (event->isAccepted() || !isDragging) return;

    event->accept();

    auto mouseDelta = event->pos() - mouseStartPoint;

    auto accuracyDelta = mouseDelta.x();
    auto motionDelta = mouseDelta.y();
    auto scaleFactor = drawBoundingRect().height();

    if (control->mode() == NodeNumControl::Mode::SLIDER_H) {
        accuracyDelta = mouseDelta.y();
        motionDelta = -mouseDelta.x();
        scaleFactor = drawBoundingRect().width();
    }

    auto accuracy = scaleFactor * 2 + (float) std::abs(accuracyDelta) * 100 / scaleFactor;
    auto delta = (float) (motionDelta / accuracy);
    setCVal(beforeDragVal.withLR(
        beforeDragVal.left - delta,
        beforeDragVal.right - delta
    ));
}

void NumControl::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseReleaseEvent(event);
    if (event->isAccepted()) return;

    event->accept();

    isDragging = false;
}

void NumControl::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseDoubleClickEvent(event);
    control->sink()->setActive(false);
    emit mouseLeave();
}

void NumControl::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink()->setActive(true);
    emit mouseEnter();
}

void NumControl::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->sink()->setActive(false);
    emit mouseLeave();
}

void NumControl::wheelEvent(QGraphicsSceneWheelEvent *event) {
    if (control->mode() == NodeNumControl::Mode::TOGGLE || control->mode() == NodeNumControl::Mode::PLUG) {
        return;
    }

    auto delta = event->delta() / 1200.f;
    auto cVal = getCVal();
    setCVal(cVal.withLR(
        cVal.left + delta,
        cVal.right + delta
    ));
}

void NumControl::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    auto clipboard = QGuiApplication::clipboard();
    // todo: make this usable with new paste system
    //float pasteVal = 0;
    //auto canPaste = AxiomUtil::strToFloat(clipboard->text(), pasteVal);

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
    auto setValAction = menu.addAction(tr("&Set Value..."));
    auto copyValAction = menu.addAction(tr("&Copy Value"));
    auto pasteValAction = menu.addAction(tr("&Paste Value"));
    //pasteValAction->setEnabled(canPaste);
    menu.addSeparator();
    auto zeroAction = menu.addAction(tr("Set to &0"));
    auto oneAction = menu.addAction(tr("Set to &1"));
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
    } else if (selectedAction == setValAction) {
        auto editor = new FloatingValueEditor(valueAsString(getCVal()), event->scenePos());
        scene()->addItem(editor);
        connect(editor, &FloatingValueEditor::valueSubmitted,
                this, &NumControl::setValue);
    } else if (selectedAction == copyValAction) {
        clipboard->setText(valueAsString(getCVal()));
    } else if (selectedAction == pasteValAction) {
        setCVal(stringAsValue(clipboard->text(), getCVal()));
    } else if (selectedAction == zeroAction) {
        setCVal(getCVal().withLR(0, 0));
    } else if (selectedAction == oneAction) {
        setCVal(getCVal().withLR(1, 1));
    } else if (selectedAction == moveAction) {
        control->select(true);
    } else if (selectedAction == nameShownAction) {
        control->setShowName(nameShownAction->isChecked());
    } else if (exposedAction && selectedAction ==  exposedAction) {
        control->node->parentSchematic->exposeControl(control);
    }
}

void NumControl::setValue(QString value) {
    setCVal(stringAsValue(value, getCVal()));
}

QString NumControl::valueAsString(MaximRuntime::NumValue num) {
    switch (control->channel()) {
        case NodeNumControl::Channel::LEFT:
            return QString::number(num.left);
        case NodeNumControl::Channel::RIGHT:
            return QString::number(num.right);
        case NodeNumControl::Channel::BOTH: {
            if (num.left == num.right) return QString::number(num.left);
            else return QString::number(num.left) + ", " + QString::number(num.right);
        }
    }
}

MaximRuntime::NumValue NumControl::stringAsValue(const QString &str, MaximRuntime::NumValue oldNum) {
    auto commaIndex = str.indexOf(',');
    auto leftStr = commaIndex >= 0 ? str.left(commaIndex) : str;
    auto rightStr = commaIndex >= 0 ? str.mid(commaIndex + 1) : str;

    float leftNum, rightNum;
    if (!AxiomUtil::strToFloat(str, leftNum)) leftNum = oldNum.left;
    if (!AxiomUtil::strToFloat(str, rightNum)) rightNum = oldNum.right;

    switch (control->channel()) {
        case NodeNumControl::Channel::LEFT:
            return oldNum.withL(leftNum);
        case NodeNumControl::Channel::RIGHT:
            return oldNum.withR(rightNum);
        case NodeNumControl::Channel::BOTH:
            return oldNum.withLR(leftNum, rightNum);
    }
}

MaximRuntime::NumValue NumControl::getCVal() const {
    auto v = control->value();
    switch (control->channel()) {
        case NodeNumControl::Channel::LEFT:
            return v.withLR(v.left, v.left);
        case NodeNumControl::Channel::RIGHT:
            return v.withLR(v.right, v.right);
        case NodeNumControl::Channel::BOTH:
            return v;
    }
}

void NumControl::setCVal(MaximRuntime::NumValue v) const {
    switch (control->channel()) {
        case NodeNumControl::Channel::LEFT:
            control->setValue(control->value().withL(v.left));
        case NodeNumControl::Channel::RIGHT:
            control->setValue(control->value().withR(v.right));
        case NodeNumControl::Channel::BOTH:
            control->setValue(control->value().withLR(v.left, v.right));
    }
}
