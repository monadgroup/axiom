#include "NumControlItem.h"

#include <QtCore/QRegularExpression>
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
#include "editor/model/actions/SetNumRangeAction.h"
#include "editor/model/actions/SetNumValueAction.h"
#include "editor/model/objects/ControlSurface.h"
#include "editor/model/objects/Node.h"
#include "editor/model/objects/NumControl.h"

using namespace AxiomGui;
using namespace AxiomModel;

static std::vector<std::pair<QString, NumControl::DisplayMode>> modes = {
    std::make_pair("&Plug", NumControl::DisplayMode::PLUG), std::make_pair("&Knob", NumControl::DisplayMode::KNOB),
    std::make_pair("&Horizontal Slider", NumControl::DisplayMode::SLIDER_H),
    std::make_pair("&Vertical Slider", NumControl::DisplayMode::SLIDER_V),
    std::make_pair("&Toggle Button", NumControl::DisplayMode::TOGGLE)};

NumControlItem::NumControlItem(NumControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _plugImage(":/icons/num-plug.png") {
    control->valueChanged.connect(this, &NumControlItem::controlValueChanged);
    control->displayModeChanged.connect(this, &NumControlItem::triggerUpdate);
    control->rangeChanged.connect(this, &NumControlItem::triggerUpdate);
    control->connections().events().itemAdded().connect(this, &NumControlItem::triggerUpdate);
    control->connections().events().itemRemoved().connect(this, &NumControlItem::triggerUpdate);

    connect(&showValueTimer, &QTimer::timeout, this, &NumControlItem::showValueExpired);
}

QRegularExpression numRegex("^\\s*(-?[e\\d\\.]+)\\s*([kmgt])*?(v|a|q|hz|b|beat|beats|ms|s|samples|sample|μ|db)?\\s*$",
                            QRegularExpression::CaseInsensitiveOption);

bool NumControlItem::unformatString(const QString &str, float *valOut, AxiomModel::FormType *formOut) {
    auto match = numRegex.match(str);
    if (!match.hasMatch()) return false;

    auto inputNum = match.captured(1);
    auto inputModifier = match.captured(2);
    auto inputForm = match.captured(3);

    float parsedNum = *valOut;
    if (!AxiomUtil::strToFloat(inputNum, parsedNum)) return false;

    if (inputModifier.compare("k", Qt::CaseInsensitive) == 0) {
        parsedNum *= 1e3;
    } else if (inputModifier.compare("m", Qt::CaseInsensitive) == 0) {
        parsedNum *= 1e6;
    } else if (inputModifier.compare("g", Qt::CaseInsensitive) == 0) {
        parsedNum *= 1e12;
    } else if (inputModifier.compare("t", Qt::CaseInsensitive) == 0) {
        parsedNum *= 1e15;
    }

    if (inputForm.compare("v", Qt::CaseInsensitive) == 0) {
        *formOut = AxiomModel::FormType::OSCILLATOR;
    } else if (inputForm.compare("a", Qt::CaseInsensitive) == 0) {
        *formOut = AxiomModel::FormType::AMPLITUDE;
    } else if (inputForm.compare("q", Qt::CaseInsensitive) == 0) {
        *formOut = AxiomModel::FormType::Q;
    } else if (inputForm.compare("hz", Qt::CaseInsensitive) == 0) {
        *formOut = AxiomModel::FormType::FREQUENCY;
    } else if (inputForm.compare("beats", Qt::CaseInsensitive) == 0 ||
               inputForm.compare("beat", Qt::CaseInsensitive) == 0 ||
               inputForm.compare("b", Qt::CaseInsensitive) == 0) {
        *formOut = AxiomModel::FormType::BEATS;
    } else if (inputForm.compare("ms", Qt::CaseInsensitive) == 0) {
        parsedNum /= 1000;
        *formOut = AxiomModel::FormType::SECONDS;
    } else if (inputForm.compare("s", Qt::CaseInsensitive) == 0) {
        *formOut = AxiomModel::FormType::SECONDS;
    } else if (inputForm.compare("μ", Qt::CaseInsensitive) == 0 ||
               inputForm.compare("samples", Qt::CaseInsensitive) == 0 ||
               inputForm.compare("sample", Qt::CaseInsensitive) == 0) {
        *formOut = AxiomModel::FormType::SAMPLES;
    } else if (inputForm.compare("db", Qt::CaseInsensitive) == 0) {
        *formOut = AxiomModel::FormType::DB;
    } else {
        *formOut = AxiomModel::FormType::NONE;
    }

    *valOut = parsedNum;
    return true;
}

void NumControlItem::paintControl(QPainter *painter) {
    auto normalizedVal = getNormalizedValue();

    auto unclampedVal =
        normalizedVal.withLR((normalizedVal.left - control->minValue()) / (control->maxValue() - control->minValue()),
                             (normalizedVal.right - control->minValue()) / (control->maxValue() - control->minValue()));

    auto clampedVal =
        normalizedVal.withLR(std::clamp(unclampedVal.left, 0.f, 1.f), std::clamp(unclampedVal.right, 0.f, 1.f));

    auto controlEnabled = control->isEnabled();
    auto normalColor = controlEnabled ? CommonColors::numNormal : CommonColors::disabledNormal;
    auto activeColor = controlEnabled ? CommonColors::numActive : CommonColors::disabledActive;

    switch (control->displayMode()) {
    case NumControl::DisplayMode::PLUG:
        plugPainter.paint(painter, aspectBoundingRect(), hoverState(), clampedVal, normalColor, _plugImage);
        break;
    case NumControl::DisplayMode::KNOB:
        knobPainter.paint(painter, aspectBoundingRect(), hoverState(), clampedVal, normalColor, activeColor);
        break;
    case NumControl::DisplayMode::SLIDER_H:
        sliderPainter.paint(painter, drawBoundingRect(), hoverState(), clampedVal, false, normalColor, activeColor);
        break;
    case NumControl::DisplayMode::SLIDER_V:
        sliderPainter.paint(painter, drawBoundingRect(), hoverState(), clampedVal, true, normalColor, activeColor);
        break;
    case NumControl::DisplayMode::TOGGLE:
        togglePainter.paint(painter, drawBoundingRect(), hoverState(), unclampedVal);
        break;
    }
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
        auto cv = getNormalizedValue();
        auto isActive = cv.left != 0 || cv.right != 0;
        auto newVal = !isActive;
        setNormalizedValue(cv.withLR(newVal, newVal));
        control->root()->history().append(
            SetNumValueAction::create(control->uuid(), cv, control->value(), control->root()));
    } else if (!isDragging) {
        isDragging = true;
        beforeDragVal = clampValue(getNormalizedValue());
        mouseStartPoint = event->pos();
    }

    canReplaceHistoryOnScroll = false;
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
    auto delta = (float) (motionDelta / accuracy * (control->maxValue() - control->minValue()));
    setNormalizedValue(clampValue(beforeDragVal.withLR(beforeDragVal.left - delta, beforeDragVal.right - delta)));
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

    auto oldValue = control->value();
    auto numClicks = event->delta() / 120.f;

    // if the control has stepping, increment/decrement by that amount - otherwise go by 1/10th
    float stepAmount;
    if (control->step()) {
        stepAmount = numClicks * (control->maxValue() - control->minValue()) / control->step();
    } else {
        stepAmount = numClicks / 10.f * (control->maxValue() - control->minValue());
    }

    auto cVal = getNormalizedValue();
    setNormalizedValue(clampValue(cVal.withLR(cVal.left + stepAmount, cVal.right + stepAmount)));
    auto newValue = control->value();

    // to avoid spamming the history list, if the previous action was a SetNumValueAction on this control, just update
    // it
    auto &history = control->root()->history();
    if (canReplaceHistoryOnScroll && history.stackPos() > 0) {
        auto lastAction = history.stack()[history.stackPos() - 1].get();
        if (auto setNumValueAction = dynamic_cast<SetNumValueAction *>(lastAction)) {
            oldValue = setNumValueAction->beforeVal();
            history.undo();
        }
    }
    history.append(SetNumValueAction::create(control->uuid(), oldValue, newValue, control->root()));

    canReplaceHistoryOnScroll = true;
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
    auto setRangeAction = menu.addAction("Set &Range...");
    auto setStepAction = menu.addAction("Set &Step...");
    menu.addSeparator();
    buildMenuEnd(menu);

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == setValAction) {
        auto editor = new FloatingValueEditor(AxiomUtil::formatNumForm(control->value(), true), event->scenePos());
        scene()->addItem(editor);
        connect(editor, &FloatingValueEditor::valueSubmitted, this, &NumControlItem::setStringValue);
    } else if (selectedAction == copyValAction) {
        clipboard->setText(AxiomUtil::formatNumForm(control->value(), true));
    } else if (selectedAction == pasteValAction) {
        setStringValue(clipboard->text());
    } else if (selectedAction == zeroAction) {
        setValue(control->value().withLR(0, 0));
    } else if (selectedAction == oneAction) {
        setValue(control->value().withLR(1, 1));
    } else if (selectedAction == setRangeAction) {
        editNumRange(false, event->scenePos());
    } else if (selectedAction == setStepAction) {
        editNumRange(true, event->scenePos());
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
    setValue(stringAsValue(value, control->value()));
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

NumValue NumControlItem::stringAsValue(const QString &str, NumValue oldNum) {
    auto separatorIndex = str.indexOf('/');
    auto leftStr = separatorIndex >= 0 ? str.left(separatorIndex) : str;
    auto rightStr = separatorIndex >= 0 ? str.mid(separatorIndex + 1) : str;

    float leftNum = oldNum.left, rightNum = oldNum.right;
    AxiomModel::FormType leftForm = oldNum.form, rightForm = oldNum.form;
    unformatString(leftStr, &leftNum, &leftForm);
    unformatString(rightStr, &rightNum, &rightForm);

    // if no form was provided, default to the current one
    if (leftForm == FormType::NONE) leftForm = rightForm;
    if (leftForm == FormType::NONE) leftForm = oldNum.form;

    return {leftNum, rightNum, leftForm};
}

AxiomModel::NumValue NumControlItem::clampValue(AxiomModel::NumValue value) {
    return value.withLR(std::clamp(value.left, control->minValue(), control->maxValue()),
                        std::clamp(value.right, control->minValue(), control->maxValue()));
}

AxiomModel::NumValue NumControlItem::getNormalizedValue() {
    auto val = control->value();
    if (control->root()->runtime()) {
        val = control->root()->runtime()->convertNum(FormType::CONTROL, val);
    }
    return val;
}

void NumControlItem::setNormalizedValue(AxiomModel::NumValue val) {
    // apply stepping
    if (control->step() > 0) {
        auto stepSize = (control->maxValue() - control->minValue()) / control->step();
        val.left = std::round(val.left / stepSize) * stepSize;
        val.right = std::round(val.right / stepSize) * stepSize;
    }

    auto currentForm = control->value().form;
    if (control->root()->runtime()) {
        val = control->root()->runtime()->convertNum(currentForm, val);
    }

    control->setValue(val);
}

QString NumControlItem::getLabelText() const {
    if ((hoverState() || isShowingValue) && !displayNameOverride) {
        return AxiomUtil::formatNumForm(control->value(), true);
    } else {
        return ControlItem::getLabelText();
    }
}

void NumControlItem::editNumRange(bool selectStep, QPointF pos) {
    // convert the min/max values to whatever form we current have
    AxiomModel::NumValue currentMinMax = {control->minValue(), control->maxValue(), AxiomModel::FormType::CONTROL};
    auto originalForm = control->value().form;
    if (control->root()->runtime()) {
        currentMinMax = control->root()->runtime()->convertNum(originalForm, currentMinMax);
    }

    auto minMaxDefaultStr =
        static_cast<QString>(AxiomUtil::formatChannelFull(currentMinMax.left, currentMinMax.form) % " - " %
                             AxiomUtil::formatChannelFull(currentMinMax.right, currentMinMax.form));
    auto stepStr = QString::number(control->step());
    QString stepSeparator = " @ ";

    int selectStart, selectEnd;
    if (selectStep) {
        selectStart = minMaxDefaultStr.size() + stepSeparator.size();
        selectEnd = selectStart + stepStr.size();
    } else {
        selectStart = 0;
        selectEnd = minMaxDefaultStr.size();
    }

    auto editor = new FloatingValueEditor(minMaxDefaultStr % stepSeparator % stepStr, pos, selectStart, selectEnd);
    scene()->addItem(editor);
    connect(editor, &FloatingValueEditor::valueSubmitted, this, [this, currentMinMax, originalForm](QString newStr) {
        auto stepSeparatorIndex = newStr.indexOf('@');

        QString rangeStr, stepStr;
        if (stepSeparatorIndex < 0) {
            rangeStr = std::move(newStr);
            stepStr = "";
        } else {
            rangeStr = newStr.left(stepSeparatorIndex);
            stepStr = newStr.mid(stepSeparatorIndex + 1);
        }

        auto minMaxSeparatorIndex = rangeStr.trimmed().indexOf('-', 1);
        auto newMin = control->minValue();
        auto newMax = control->maxValue();
        if (minMaxSeparatorIndex > 0) {
            auto minStr = rangeStr.left(minMaxSeparatorIndex);
            auto maxStr = rangeStr.mid(minMaxSeparatorIndex + 1);

            float minValue = currentMinMax.left, maxValue = currentMinMax.right;
            AxiomModel::FormType minForm = originalForm, maxForm = originalForm;

            if (!unformatString(minStr, &minValue, &minForm)) return;
            if (!unformatString(maxStr, &maxValue, &maxForm)) return;

            // convert back to CONTROL space
            AxiomModel::NumValue newMinMax = {minValue, maxValue, minForm};
            if (control->root()->runtime()) {
                newMinMax = control->root()->runtime()->convertNum(AxiomModel::FormType::CONTROL, newMinMax);
            }

            // swap values if necessary
            if (newMinMax.left > newMinMax.right) {
                std::swap(newMinMax.left, newMinMax.right);
            }

            newMin = newMinMax.left;
            newMax = newMinMax.right;
        }

        bool stepValid;
        uint32_t newStep = (uint32_t) stepStr.trimmed().toUInt(&stepValid);
        if (!stepValid) newStep = control->step();

        if (newMin != control->minValue() || newMax != control->maxValue() || newStep != control->step()) {
            control->root()->history().append(SetNumRangeAction::create(control->uuid(), control->minValue(),
                                                                        control->maxValue(), control->step(), newMin,
                                                                        newMax, newStep, control->root()));
        }
    });
}
