#include "NumControlItem.h"

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>

#include "editor/model/Project.h"
#include "editor/model/objects/NumControl.h"
#include "editor/model/actions/SetNumModeAction.h"
#include "editor/model/actions/SetNumValueAction.h"
#include "../node/NodeItem.h"
#include "../CommonColors.h"
#include "../FloatingValueEditor.h"
#include "../../util.h"

using namespace AxiomGui;
using namespace AxiomModel;

static std::vector<std::pair<QString, NumControl::DisplayMode>> modes = {
    std::make_pair("&Plug", NumControl::DisplayMode::PLUG),
    std::make_pair("&Knob", NumControl::DisplayMode::KNOB),
    std::make_pair("&Horizontal Slider", NumControl::DisplayMode::SLIDER_H),
    std::make_pair("&Vertical Slider", NumControl::DisplayMode::SLIDER_V),
    std::make_pair("&Toggle Button", NumControl::DisplayMode::TOGGLE)
};

NumControlItem::NumControlItem(NumControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
    control->valueChanged.connect(this, &NumControlItem::triggerUpdate);
    control->displayModeChanged.connect(this, &NumControlItem::triggerUpdate);
    control->connections().itemAdded.connect(this, &NumControlItem::triggerUpdate);
    control->connections().itemRemoved.connect(this, &NumControlItem::triggerUpdate);
}

void NumControlItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    ControlItem::paint(painter, option, widget);

    switch (control->displayMode()) {
        case NumControl::DisplayMode::PLUG:
            plugPainter.paint(painter, aspectBoundingRect(), hoverState());
            break;
        case NumControl::DisplayMode::KNOB:
            knobPainter.paint(painter, aspectBoundingRect(), hoverState(), getCVal(), CommonColors::numNormal,
                              CommonColors::numActive);
            break;
        case NumControl::DisplayMode::SLIDER_H:
            sliderPainter.paint(painter, drawBoundingRect(), hoverState(), getCVal(), false, CommonColors::numNormal,
                                CommonColors::numActive);
            break;
        case NumControl::DisplayMode::SLIDER_V:
            sliderPainter.paint(painter, drawBoundingRect(), hoverState(), getCVal(), true, CommonColors::numNormal,
                                CommonColors::numActive);
            break;
        case NumControl::DisplayMode::TOGGLE:
            togglePainter.paint(painter, drawBoundingRect(), hoverState(), getCVal());
            break;
    }
}

QPainterPath NumControlItem::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();
    return controlPath();
}

QRectF NumControlItem::useBoundingRect() const {
    switch (control->displayMode()) {
        case NumControl::DisplayMode::PLUG:
            return plugPainter.getBounds(aspectBoundingRect());
        case NumControl::DisplayMode::KNOB:
            return knobPainter.getBounds(aspectBoundingRect());
        case NumControl::DisplayMode::SLIDER_H:
            return sliderPainter.getBounds(drawBoundingRect(), false);
        case NumControl::DisplayMode::SLIDER_V:
            return sliderPainter.getBounds(drawBoundingRect(), true);
        case NumControl::DisplayMode::TOGGLE:
            return togglePainter.getBounds(drawBoundingRect());
    }
    unreachable;
}

QPainterPath NumControlItem::controlPath() const {
    QPainterPath path;
    switch (control->displayMode()) {
        case NumControl::DisplayMode::PLUG:
            plugPainter.shape(path, aspectBoundingRect());
            break;
        case NumControl::DisplayMode::KNOB:
            knobPainter.shape(path, aspectBoundingRect());
            break;
        case NumControl::DisplayMode::SLIDER_H:
            sliderPainter.shape(path, drawBoundingRect(), false);
            break;
        case NumControl::DisplayMode::SLIDER_V:
            sliderPainter.shape(path, drawBoundingRect(), true);
            break;
        case NumControl::DisplayMode::TOGGLE:
            togglePainter.shape(path, drawBoundingRect());
            break;
    }
    return path;
}

void NumControlItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mousePressEvent(event);
    if (event->isAccepted() || event->button() != Qt::LeftButton ||
        control->displayMode() == NumControl::DisplayMode::PLUG) {
        return;
    }
    event->accept();

    if (control->displayMode() == NumControl::DisplayMode::TOGGLE) {
        auto cv = getCVal();
        auto isActive = cv.left != 0 || cv.right != 0;
        auto newVal = !isActive;
        setCVal(cv.withLR(newVal, newVal));
    } else if (!isDragging) {
        isDragging = true;
        beforeDragVal = getCVal();
        mouseStartPoint = event->pos();
    }
}

void NumControlItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseMoveEvent(event);
    if (event->isAccepted() || !isDragging) return;

    event->accept();

    auto mouseDelta = event->pos() - mouseStartPoint;

    auto accuracyDelta = mouseDelta.x();
    auto motionDelta = mouseDelta.y();
    auto scaleFactor = drawBoundingRect().height();

    if (control->displayMode() == NumControl::DisplayMode::SLIDER_H) {
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

void NumControlItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseReleaseEvent(event);
    if (event->isAccepted()) return;

    event->accept();

    if (isDragging) {
        isDragging = false;
        if (control->value() != beforeDragVal) {
            control->root()->history().append(SetNumValueAction::create(control->uuid(), beforeDragVal, control->value(), control->root()));
        }
    }
}

void NumControlItem::wheelEvent(QGraphicsSceneWheelEvent *event) {
    if (control->displayMode() == NumControl::DisplayMode::TOGGLE ||
        control->displayMode() == NumControl::DisplayMode::PLUG) {
        return;
    }

    auto delta = event->delta() / 1200.f;
    auto cVal = getCVal();
    setCVal(cVal.withLR(
        cVal.left + delta,
        cVal.right + delta
    ));
}

void NumControlItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    auto clipboard = QGuiApplication::clipboard();

    QMenu menu;
    //auto clearAction = menu.addAction("&Clear Connections");

    auto modeMenu = menu.addMenu("&Display as...");
    for (const auto &modePair : modes) {
        auto action = modeMenu->addAction(modePair.first);
        action->setCheckable(true);
        action->setChecked(control->displayMode() == modePair.second);

        connect(action, &QAction::triggered,
                [this, modePair]() {
                    control->root()->history().append(SetNumModeAction::create(control->uuid(), control->displayMode(), modePair.second, control->root()));
                });
    }

    menu.addSeparator();
    auto setValAction = menu.addAction("&Set Value...");
    auto copyValAction = menu.addAction("&Copy Value");
    auto pasteValAction = menu.addAction("&Paste Value");
    menu.addSeparator();
    auto zeroAction = menu.addAction("Set to &0");
    auto oneAction = menu.addAction("Set to &1");

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == setValAction) {
        auto editor = new FloatingValueEditor(valueAsString(getCVal()), event->scenePos());
        scene()->addItem(editor);
        connect(editor, &FloatingValueEditor::valueSubmitted,
                this, &NumControlItem::setStringValue);
    } else if (selectedAction == copyValAction) {
        clipboard->setText(valueAsString(getCVal()));
    } else if (selectedAction == pasteValAction) {
        setStringValue(clipboard->text());
    } else if (selectedAction == zeroAction) {
        setValue(control->value().withLR(0, 0));
    } else if (selectedAction == oneAction) {
        setValue(control->value().withLR(1, 1));
    }

    // todo
    /*auto clipboard = QGuiApplication::clipboard();
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
                    DO_ACTION(control->node->parentSchematic->project()->history, HistoryList::ActionType::CHANGE_MODE,
                              {
                                  control->setMode(modePair.second);
                              });
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
        DO_ACTION(control->node->parentSchematic->project()->history, HistoryList::ActionType::DISCONNECT_ALL, {
            control->sink()->clearConnections();
        });
    } else if (selectedAction == setValAction) {
        auto editor = new FloatingValueEditor(valueAsString(getCVal()), event->scenePos());
        scene()->addItem(editor);
        connect(editor, &FloatingValueEditor::valueSubmitted,
                this, &NumControlItem::setValue);
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
        auto actionType = nameShownAction->isChecked() ? HistoryList::ActionType::SHOW_CONTROL_NAME : HistoryList::ActionType::HIDE_CONTROL_NAME;
        DO_ACTION(control->node->parentSchematic->project()->history, actionType, {
            control->setShowName(nameShownAction->isChecked());
        });
    } else if (exposedAction && selectedAction == exposedAction) {
        control->node->parentSchematic->exposeControl(control);
    }*/
}

void NumControlItem::setStringValue(QString value) {
    setValue(stringAsValue(value, getCVal()));
}

void NumControlItem::setValue(MaximRuntime::NumValue value) {
    if (value != control->value()) {
        control->root()->history().append(SetNumValueAction::create(control->uuid(), control->value(), value, control->root()));
    }
}

QString NumControlItem::valueAsString(MaximRuntime::NumValue num) {
    switch (control->channel()) {
        case NumControl::Channel::LEFT:
            return QString::number(num.left);
        case NumControl::Channel::RIGHT:
            return QString::number(num.right);
        case NumControl::Channel::BOTH: {
            if (num.left == num.right) return QString::number(num.left);
            else return QString::number(num.left) + ", " + QString::number(num.right);
        }
    }
    unreachable;
}

MaximRuntime::NumValue NumControlItem::stringAsValue(const QString &str, MaximRuntime::NumValue oldNum) {
    auto commaIndex = str.indexOf(',');
    auto leftStr = commaIndex >= 0 ? str.left(commaIndex) : str;
    auto rightStr = commaIndex >= 0 ? str.mid(commaIndex + 1) : str;

    float leftNum, rightNum;
    if (!AxiomUtil::strToFloat(str, leftNum)) leftNum = oldNum.left;
    if (!AxiomUtil::strToFloat(str, rightNum)) rightNum = oldNum.right;

    switch (control->channel()) {
        case NumControl::Channel::LEFT:
            return oldNum.withL(leftNum);
        case NumControl::Channel::RIGHT:
            return oldNum.withR(rightNum);
        case NumControl::Channel::BOTH:
            return oldNum.withLR(leftNum, rightNum);
    }
    unreachable;
}

MaximRuntime::NumValue NumControlItem::getCVal() const {
    auto v = control->value();
    switch (control->channel()) {
        case NumControl::Channel::LEFT:
            return v.withLR(v.left, v.left);
        case NumControl::Channel::RIGHT:
            return v.withLR(v.right, v.right);
        case NumControl::Channel::BOTH:
            return v;
    }
    unreachable;
}

void NumControlItem::setCVal(MaximRuntime::NumValue v) const {
    switch (control->channel()) {
        case NumControl::Channel::LEFT:
            control->setValue(control->value().withL(v.left));
        case NumControl::Channel::RIGHT:
            control->setValue(control->value().withR(v.right));
        case NumControl::Channel::BOTH:
            control->setValue(control->value().withLR(v.left, v.right));
    }
}
