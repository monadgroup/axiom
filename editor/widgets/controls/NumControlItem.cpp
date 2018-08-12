#include "NumControlItem.h"

#include <QtCore/QStringBuilder>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <cmath>

#include "../../util.h"
#include "../CommonColors.h"
#include "../FloatingValueEditor.h"
#include "../node/NodeItem.h"
#include "editor/compiler/interface/Runtime.h"
#include "editor/model/ModelRoot.h"
#include "editor/model/Project.h"
#include "editor/model/actions/SetNumModeAction.h"
#include "editor/model/actions/SetNumValueAction.h"
#include "editor/model/objects/NumControl.h"

using namespace AxiomGui;
using namespace AxiomModel;

static std::vector<std::pair<QString, NumControl::DisplayMode>> modes = {
    std::make_pair("&Plug", NumControl::DisplayMode::PLUG), std::make_pair("&Knob", NumControl::DisplayMode::KNOB),
    std::make_pair("&Horizontal Slider", NumControl::DisplayMode::SLIDER_H),
    std::make_pair("&Vertical Slider", NumControl::DisplayMode::SLIDER_V),
    std::make_pair("&Toggle Button", NumControl::DisplayMode::TOGGLE)};

NumControlItem::NumControlItem(NumControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
    control->valueChanged.connect(this, &NumControlItem::controlValueChanged);
    control->displayModeChanged.connect(this, &NumControlItem::triggerUpdate);
    control->connections().itemAdded.connect(this, &NumControlItem::triggerUpdate);
    control->connections().itemRemoved.connect(this, &NumControlItem::triggerUpdate);

    connect(&showValueTimer, &QTimer::timeout, this, &NumControlItem::showValueExpired);
}

static std::array<QString, 12> noteNames{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

static QString getNoteName(float noteVal) {
    auto intNote = static_cast<size_t>(noteVal);
    auto noteNameIndex = intNote % noteNames.size();
    auto octave = intNote / noteNames.size();

    return QString::number(intNote) % " " % noteNames[noteNameIndex] % QString::number(octave);
}

QString NumControlItem::formatNumber(float val, AxiomModel::FormType form) {
    switch (form) {
    case AxiomModel::FormType::NONE:
    case AxiomModel::FormType::CONTROL:
        return QString::number(val, 'f', 2);
    case AxiomModel::FormType::OSCILLATOR:
        return static_cast<QString>(QString::number(val, 'f', 2) % " V");
    case AxiomModel::FormType::AMPLITUDE:
        return static_cast<QString>(QString::number(val, 'f', 2) % " A");
    case AxiomModel::FormType::Q:
        return static_cast<QString>(QString::number(val, 'f', 1) % " Q");
    case AxiomModel::FormType::NOTE:
        return getNoteName(val);
    case AxiomModel::FormType::FREQUENCY:
        return val < 1000 ? static_cast<QString>(QString::number(val, 'f', 2) % " Hz")
                          : static_cast<QString>(QString::number(val / 1000, 'f', 2) % "KHz");
    case AxiomModel::FormType::BEATS:
        return QString::number(val, 'f', 1) % " beats";
    case AxiomModel::FormType::SECONDS:
        return val < 0.1 ? static_cast<QString>(QString::number(val * 1000, 'f', 2) % " ms")
                         : static_cast<QString>(QString::number(val, 'f', 2) % " s");
    case AxiomModel::FormType::SAMPLES:
        return static_cast<QString>(QString::number((int) val) % " μ");
    case AxiomModel::FormType::DB:
        return QString::number(val, 'f', 1) % " dB";
    }
    unreachable;
}

void NumControlItem::paintControl(QPainter *painter) {
    switch (control->displayMode()) {
    case NumControl::DisplayMode::PLUG:
        plugPainter.paint(painter, aspectBoundingRect(), hoverState(), getCVal(), CommonColors::numNormal);
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

bool NumControlItem::showLabelInCenter() const {
    switch (control->displayMode()) {
    case NumControl::DisplayMode::PLUG:
    case NumControl::DisplayMode::KNOB:
    case NumControl::DisplayMode::TOGGLE:
        return true;
    case NumControl::DisplayMode::SLIDER_H:
    case NumControl::DisplayMode::SLIDER_V:
        return false;
    }
    unreachable;
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
    if (!isEditable() || event->button() != Qt::LeftButton || control->displayMode() == NumControl::DisplayMode::PLUG) {
        return;
    }
    event->accept();

    if (control->displayMode() == NumControl::DisplayMode::TOGGLE) {
        auto cv = getCVal();
        auto isActive = cv.left != 0 || cv.right != 0;
        auto newVal = !isActive;
        setCVal(cv.withLR(newVal, newVal));
        control->root()->history().append(
            SetNumValueAction::create(control->uuid(), cv, control->value(), control->root()));
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
    setCVal(beforeDragVal.withLR(beforeDragVal.left - delta, beforeDragVal.right - delta));
}

void NumControlItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mouseReleaseEvent(event);
    if (event->isAccepted()) return;

    event->accept();

    if (isDragging) {
        isDragging = false;
        if (control->value() != beforeDragVal) {
            control->root()->history().append(
                SetNumValueAction::create(control->uuid(), beforeDragVal, control->value(), control->root()));
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
    setCVal(cVal.withLR(cVal.left + delta, cVal.right + delta));
}

void NumControlItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    auto clipboard = QGuiApplication::clipboard();

    QMenu menu;
    buildMenuStart(menu);

    auto modeMenu = menu.addMenu("&Display as...");
    for (const auto &modePair : modes) {
        auto action = modeMenu->addAction(modePair.first);
        action->setCheckable(true);
        action->setChecked(control->displayMode() == modePair.second);

        connect(action, &QAction::triggered, [this, modePair]() {
            control->root()->history().append(
                SetNumModeAction::create(control->uuid(), control->displayMode(), modePair.second, control->root()));
        });
    }

    menu.addSeparator();
    auto setValAction = menu.addAction("&Set Value...");
    auto copyValAction = menu.addAction("&Copy Value");
    auto pasteValAction = menu.addAction("&Paste Value");
    menu.addSeparator();
    auto zeroAction = menu.addAction("Set to &0");
    auto oneAction = menu.addAction("Set to &1");
    menu.addSeparator();
    buildMenuEnd(menu);

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == setValAction) {
        auto editor = new FloatingValueEditor(valueAsString(getCVal()), event->scenePos());
        scene()->addItem(editor);
        connect(editor, &FloatingValueEditor::valueSubmitted, this, &NumControlItem::setStringValue);
    } else if (selectedAction == copyValAction) {
        clipboard->setText(valueAsString(getCVal()));
    } else if (selectedAction == pasteValAction) {
        setStringValue(clipboard->text());
    } else if (selectedAction == zeroAction) {
        setValue(control->value().withLR(0, 0));
    } else if (selectedAction == oneAction) {
        setValue(control->value().withLR(1, 1));
    }
}

void NumControlItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (isShowingValue && !control->name().isEmpty()) {
        displayNameOverride = true;
    }

    ControlItem::hoverEnterEvent(event);
}

void NumControlItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    displayNameOverride = false;
    ControlItem::hoverLeaveEvent(event);
}

void NumControlItem::setStringValue(QString value) {
    setValue(stringAsValue(value, getCVal()));
}

void NumControlItem::controlValueChanged() {
    if (!displayNameOverride) {
        isShowingValue = true;
        showValueTimer.start(1000);
    }
    update();
}

void NumControlItem::showValueExpired() {
    isShowingValue = false;
    showValueTimer.stop();
    update();
}

void NumControlItem::setValue(NumValue value) {
    if (value != control->value()) {
        control->root()->history().append(
            SetNumValueAction::create(control->uuid(), control->value(), value, control->root()));
    }
}

QString NumControlItem::valueAsString(NumValue num) {
    switch (control->channel()) {
    case NumControl::Channel::LEFT:
        return QString::number(num.left);
    case NumControl::Channel::RIGHT:
        return QString::number(num.right);
    case NumControl::Channel::BOTH: {
        if (num.left == num.right)
            return QString::number(num.left);
        else
            return QString::number(num.left) + ", " + QString::number(num.right);
    }
    }
    unreachable;
}

NumValue NumControlItem::stringAsValue(const QString &str, NumValue oldNum) {
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

NumValue NumControlItem::clampVal(const NumValue &val) {
    return val.withLR(val.left < 0 ? 0 : val.left > 1 ? 1 : val.left,
                      val.right < 0 ? 0 : val.right > 1 ? 1 : val.right);
}

NumValue NumControlItem::getCVal() const {
    auto v = control->value();
    if (control->root()->runtime()) {
        v = control->root()->runtime()->convertNum(FormType::CONTROL, v);
    }
    v = clampVal(v);

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

void NumControlItem::setCVal(NumValue v) const {
    NumValue setVal;

    switch (control->channel()) {
    case NumControl::Channel::LEFT:
        setVal = control->value().withL(v.left);
        break;
    case NumControl::Channel::RIGHT:
        setVal = control->value().withR(v.right);
        break;
    case NumControl::Channel::BOTH:
        setVal = control->value().withLR(v.left, v.right);
        break;
    }

    setVal = clampVal(setVal).withForm(FormType::CONTROL);
    control->setValue(setVal);
}

QString NumControlItem::getLabelText() const {
    if ((hoverState() || isShowingValue) && !displayNameOverride) {
        auto controlVal = control->value();
        if (fabsf(controlVal.left - controlVal.right) < 0.01) {
            return formatNumber(controlVal.left, controlVal.form);
        } else {
            return QString("%1 / %2").arg(formatNumber(controlVal.left, controlVal.form),
                                          formatNumber(controlVal.right, controlVal.form));
        }
    } else {
        return ControlItem::getLabelText();
    }
}
