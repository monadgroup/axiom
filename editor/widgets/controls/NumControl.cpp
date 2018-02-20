#include "NumControl.h"

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
#include "editor/model/control/NodeNumControl.h"
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

QRectF NumControl::aspectBoundingRect() const {
    auto bound = drawBoundingRect();
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

void NumControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    switch (control->mode()) {
        case NodeNumControl::Mode::PLUG:
            paintPlug(painter);
            break;
        case NodeNumControl::Mode::KNOB:
            paintKnob(painter);
            break;
        case NodeNumControl::Mode::SLIDER_H:
            paintSlider(painter, false);
            break;
        case NodeNumControl::Mode::SLIDER_V:
            paintSlider(painter, true);
            break;
        case NodeNumControl::Mode::TOGGLE:
            paintToggle(painter);
            break;
    }

    ControlItem::paint(painter, option, widget);

    control->doRuntimeUpdate();
}

QPainterPath NumControl::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();

    QPainterPath path;
    switch (control->mode()) {
        case NodeNumControl::Mode::PLUG:
            path.addEllipse(getPlugBounds());
            break;
        case NodeNumControl::Mode::KNOB:
            path.addEllipse(getKnobBounds());
            break;
        case NodeNumControl::Mode::SLIDER_H:
            path.addRect(getSliderBounds(false));
            break;
        case NodeNumControl::Mode::SLIDER_V:
            path.addRect(getSliderBounds(true));
            break;
        case NodeNumControl::Mode::TOGGLE:
            path.addRect(getToggleBounds());
            break;
    }
    return path;
}

void NumControl::setHoverState(float newHoverState) {
    if (newHoverState != m_hoverState) {
        m_hoverState = newHoverState;
        update();
    }
}

QRectF NumControl::useBoundingRect() const {
    switch (control->mode()) {
        case NodeNumControl::Mode::PLUG: return getPlugBounds();
        case NodeNumControl::Mode::KNOB: return getKnobBounds();
        case NodeNumControl::Mode::SLIDER_H: return getSliderBounds(false);
        case NodeNumControl::Mode::SLIDER_V: return getSliderBounds(true);
        case NodeNumControl::Mode::TOGGLE: return getToggleBounds();
    }
}

void NumControl::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    ControlItem::mousePressEvent(event);
    if (event->isAccepted() || event->button() != Qt::LeftButton) return;

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
    auto plugModeAction = modeMenu->addAction(tr("&Plug"));
    auto knobModeAction = modeMenu->addAction(tr("&Knob"));
    auto hSliderModeAction = modeMenu->addAction(tr("&Horizontal Slider"));
    auto vSliderModeAction = modeMenu->addAction(tr("&Vertical Slider"));
    auto toggleModeAction = modeMenu->addAction(tr("&Toggle Button"));

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

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == clearAction) {
        control->sink()->clearConnections();
    } else if (selectedAction == plugModeAction) {
        control->setMode(NodeNumControl::Mode::PLUG);
    } else if (selectedAction == knobModeAction) {
        control->setMode(NodeNumControl::Mode::KNOB);
    } else if (selectedAction == hSliderModeAction) {
        control->setMode(NodeNumControl::Mode::SLIDER_H);
    } else if (selectedAction == vSliderModeAction) {
        control->setMode(NodeNumControl::Mode::SLIDER_V);
    } else if (selectedAction == toggleModeAction) {
        control->setMode(NodeNumControl::Mode::TOGGLE);
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
    }
}

void NumControl::setValue(QString value) {
    setCVal(stringAsValue(value, getCVal()));
}

QRectF NumControl::getPlugBounds() const {
    auto br = aspectBoundingRect();
    auto scaledMargin = 0.1f * br.width();
    return br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}

QRectF NumControl::getKnobBounds() const {
    auto br = aspectBoundingRect();
    auto scaledMargin = 0.1f * br.width();
    return br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}

QRectF NumControl::getSliderBounds(bool vertical) const {
    auto br = flip(drawBoundingRect(), vertical);
    auto scaledMargin = 0.1f * br.height();
    auto barHeight = br.height() / 2;
    auto barY = br.center().y() - barHeight / 2;
    return flip(QRectF(QPointF(br.x() + scaledMargin,
                               barY),
                       QSizeF(br.width() - scaledMargin * 2,
                              barHeight)), vertical);
}

QRectF NumControl::getToggleBounds() const {
    auto br = drawBoundingRect();
    auto hMargin = 0.1f * br.width();
    auto vMargin = 0.1f * br.height();
    return br.marginsRemoved(QMarginsF(hMargin, vMargin, hMargin, vMargin));
}

void NumControl::paintPlug(QPainter *painter) {
    auto scaledBorder = 0.06f * aspectBoundingRect().width();
    auto externBr = getPlugBounds();

    auto marginF = QMarginsF(scaledBorder / 2, scaledBorder / 2, scaledBorder / 2, scaledBorder / 2);

    auto baseColor = QColor(45, 45, 45);
    auto activeColor = QColor(60, 60, 60);
    auto connectedActiveColor = CommonColors::numWireActive;
    auto connectedColor = CommonColors::numWireNormal;

    if (!control->sink()->connections().empty()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(AxiomUtil::mixColor(connectedColor, connectedActiveColor, control->sink()->active())));
        painter->drawEllipse(externBr.marginsAdded(marginF));
    }

    painter->setPen(QPen(QColor(0, 0, 0), scaledBorder));
    painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, m_hoverState)));
    painter->drawEllipse(externBr.marginsRemoved(marginF));
}

void NumControl::paintKnob(QPainter *painter) {
    auto aspectWidth = aspectBoundingRect().width();
    auto scaledThickness = (0.06f + 0.04f * m_hoverState) * aspectWidth;
    auto outerBr = getKnobBounds();
    auto ringBr = outerBr.marginsRemoved(
            QMarginsF(scaledThickness / 2, scaledThickness / 2, scaledThickness / 2, scaledThickness / 2));

    auto startAngle = 240 * 16;
    auto completeAngle = -300 * 16;

    auto cv = getCVal();
    auto minVal = std::min(cv.left, cv.right);
    auto maxVal = std::max(cv.left, cv.right);

    auto baseColor = QColor(141, 141, 141);
    auto activeColor = CommonColors::controlActive;
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, m_hoverState);
    auto darkerCurrent = currentColor.darker();

    // draw background
    painter->setPen(Qt::NoPen);
    if (!control->sink()->connections().empty()) {
        auto activeBorderThickness = 0.02 * aspectWidth;
        painter->setBrush(QBrush(AxiomUtil::mixColor(CommonColors::numWireNormal, CommonColors::numWireActive, control->sink()->active())));
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
        if (markerVal < minVal || (markerVal == 1 && minVal == 1)) {
            activeMarkerPen.setColor(currentColor);
        } else if (markerVal < maxVal || (markerVal == 1 && maxVal == 1)) {
            activeMarkerPen.setColor(darkerCurrent);
        } else {
            activeMarkerPen.setColor(QColor(0, 0, 0));
        }
        painter->setPen(activeMarkerPen);

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

    painter->drawArc(ringBr, startAngle + completeAngle * maxVal,
                     completeAngle - completeAngle * maxVal);

    // draw max ring
    pen.setColor(darkerCurrent);
    painter->setPen(pen);
    painter->drawArc(ringBr, startAngle + completeAngle * minVal,
                             completeAngle * maxVal - completeAngle * minVal);

    // draw min ring
    pen.setColor(currentColor);
    painter->setPen(pen);
    painter->drawArc(ringBr, startAngle, completeAngle * minVal);
}

void NumControl::paintSlider(QPainter *painter, bool vertical) {
    auto br = flip(getSliderBounds(vertical), vertical);
    auto scaledThickness = (0.12f + 0.08f * m_hoverState) * br.height();

    auto cv = getCVal();
    auto minVal = std::min(cv.left, cv.right);
    auto maxVal = std::max(cv.left, cv.right);

    auto baseColor = QColor(141, 141, 141);
    auto activeColor = CommonColors::controlActive;
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, m_hoverState);
    auto darkerCurrent = currentColor.darker();

    // draw background
    painter->setPen(Qt::NoPen);
    if (!control->sink()->connections().empty()) {
        auto activeBorderThickness = 0.04 * br.height();
        painter->setBrush(QBrush(AxiomUtil::mixColor(CommonColors::numWireNormal, CommonColors::numWireActive, control->sink()->active())));
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
        if (markerVal < minVal || (markerVal == 1 && minVal == 1)) {
            activeMarkerPen.setColor(currentColor);
        } else if (markerVal < maxVal || (markerVal == 1 && maxVal == 1)) {
            activeMarkerPen.setColor(currentColor);
        } else {
            activeMarkerPen.setColor(QColor(0, 0, 0));
        }
        painter->setPen(activeMarkerPen);

        auto markerX = br.left() + br.width() * (vertical ? 1 - markerVal : markerVal);

        auto shiftAmt = 2.5;
        if (i % 2 == 0) shiftAmt = 2;
        if (i == 0 || i == markerCount || i == markerCount / 2) shiftAmt = 1.5;
        painter->drawLine(
                flip(QPointF(markerX, br.y() + 1), vertical),
                flip(QPointF(markerX, br.y() + br.height() / shiftAmt), vertical)
        );
    }

    auto minX = br.left() + br.width() * (vertical ? 1 - minVal : minVal);
    auto maxX = br.left() + br.width() * (vertical ? 1 - maxVal : maxVal);

    auto posY = br.y() + scaledThickness / 2;
    auto leftPos = QPointF(vertical ? br.right() : br.left(), posY);
    auto rightPos = QPointF(vertical ? br.left() : br.right(), posY);
    auto minPos = QPointF(minX, posY);
    auto maxPos = QPointF(maxX, posY);

    // draw background bar
    auto pen = QPen(QColor(0, 0, 0), scaledThickness);
    pen.setCapStyle(Qt::FlatCap);
    painter->setPen(pen);
    painter->drawLine(
            flip(maxPos, vertical),
            flip(rightPos, vertical)
    );

    // draw max bar
    pen.setColor(darkerCurrent);
    painter->setPen(pen);
    painter->drawLine(
        flip(minPos, vertical),
        flip(maxPos, vertical)
    );

    // draw min bar
    pen.setColor(currentColor);
    painter->setPen(pen);
    painter->drawLine(
            flip(leftPos, vertical),
            flip(minPos, vertical)
    );
}

void NumControl::paintToggle(QPainter *painter) {
    auto br = getToggleBounds();

    auto scaleFactor = std::hypot(br.right() - br.left(), br.bottom() - br.top());
    auto borderMargin = QMarginsF(0.02 * br.width(), 0.02 * br.height(), 0.02 * br.width(), 0.02 * br.height());

    auto baseColor = QColor(45, 45, 45);
    auto activeColor = QColor(60, 60, 60);
    auto connectedColor = CommonColors::numWireNormal;
    auto connectedActiveColor = CommonColors::numWireActive;

    auto cv = getCVal();
    auto brightness = (cv.left + cv.right) / 2;

    // draw background
    if (!control->sink()->connections().empty()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(AxiomUtil::mixColor(connectedColor, connectedActiveColor, control->sink()->active())));
        painter->drawRect(br.marginsAdded(borderMargin));
    }

    painter->setPen(QPen(QColor(0, 0, 0), 0.02 * scaleFactor));
    painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, m_hoverState)));
    painter->drawRect(br.marginsRemoved(borderMargin));

    // draw light
    auto lightRadius = 0.05 * scaleFactor;
    auto lightPos = QPointF(br.left() + br.width() / 2, br.top() + br.height() / 4);

    painter->setPen(Qt::NoPen);

    auto activeAlpha = QColor(CommonColors::controlActive.red(), CommonColors::controlActive.green(), CommonColors::controlActive.blue(), (int) (brightness * 255));
    auto glowRadius = lightRadius * 2;
    QRadialGradient gradient(lightPos, glowRadius);
    gradient.setColorAt(0, activeAlpha);
    gradient.setColorAt(1, QColor(activeAlpha.red(), activeAlpha.green(),
                                  activeAlpha.blue(), 0));
    painter->setBrush(gradient);
    painter->drawEllipse(lightPos, glowRadius, glowRadius);

    painter->setBrush(QBrush(AxiomUtil::mixColor(Qt::black, CommonColors::controlActive, brightness)));
    painter->drawEllipse(lightPos, lightRadius, lightRadius);
}

QString NumControl::valueAsString(AxiomModel::NumValue num) {
    switch (control->channel()) {
        case NodeNumControl::Channel::LEFT: return QString::number(num.left);
        case NodeNumControl::Channel::RIGHT: return QString::number(num.right);
        case NodeNumControl::Channel::BOTH: {
            if (num.left == num.right) return QString::number(num.left);
            else return QString::number(num.left) + ", " + QString::number(num.right);
        }
    }
}

AxiomModel::NumValue NumControl::stringAsValue(const QString &str, AxiomModel::NumValue oldNum) {
    auto commaIndex = str.indexOf(',');
    auto leftStr = commaIndex >= 0 ? str.left(commaIndex) : str;
    auto rightStr = commaIndex >= 0 ? str.mid(commaIndex + 1) : str;

    float leftNum, rightNum;
    if (!AxiomUtil::strToFloat(str, leftNum)) leftNum = oldNum.left;
    if (!AxiomUtil::strToFloat(str, rightNum)) rightNum = oldNum.right;

    switch (control->channel()) {
        case NodeNumControl::Channel::LEFT: return oldNum.withL(leftNum);
        case NodeNumControl::Channel::RIGHT: return oldNum.withR(rightNum);
        case NodeNumControl::Channel::BOTH: return oldNum.withLR(leftNum, rightNum);
    }
}

AxiomModel::NumValue NumControl::getCVal() const {
    auto v = control->value();
    switch (control->channel()) {
        case NodeNumControl::Channel::LEFT: return v.withLR(v.left, v.left);
        case NodeNumControl::Channel::RIGHT: return v.withLR(v.right, v.right);
        case NodeNumControl::Channel::BOTH: return v;
    }
}

void NumControl::setCVal(AxiomModel::NumValue v) const {
    switch (control->channel()) {
        case NodeNumControl::Channel::LEFT: control->setValue(control->value().withL(v.left));
        case NodeNumControl::Channel::RIGHT: control->setValue(control->value().withR(v.right));
        case NodeNumControl::Channel::BOTH: control->setValue(control->value().withLR(v.left, v.right));
    }
}
