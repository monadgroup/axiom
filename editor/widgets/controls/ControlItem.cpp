#include "ControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <QtCore/QStateMachine>
#include <QtCore/QSignalTransition>
#include <QtCore/QPropertyAnimation>

#include "editor/model/Project.h"
#include "editor/model/objects/Control.h"
#include "editor/model/objects/ControlSurface.h"
#include "editor/util.h"
#include "../ItemResizer.h"
#include "../CommonColors.h"
#include "../surface/NodeSurfaceCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

ControlItem::ControlItem(Control *control, NodeSurfaceCanvas *canvas) : control(control), canvas(canvas) {
    setAcceptHoverEvents(true);

    control->posChanged.listen(this, &ControlItem::setPos);
    control->beforeSizeChanged.listen(this, &ControlItem::triggerGeometryChange);
    control->sizeChanged.listen(this, &ControlItem::setSize);
    control->selectedChanged.listen(this, &ControlItem::updateSelected);
    control->isActiveChanged.listen(this, &ControlItem::triggerUpdate);
    control->removed.listen(this, &ControlItem::remove);

    // create resize items
    if (control->isResizable()) {
        ItemResizer::Direction directions[] = {
            ItemResizer::TOP, ItemResizer::RIGHT, ItemResizer::BOTTOM, ItemResizer::LEFT,
            ItemResizer::TOP_RIGHT, ItemResizer::TOP_LEFT, ItemResizer::BOTTOM_RIGHT, ItemResizer::BOTTOM_LEFT
        };
        for (auto i = 0; i < 8; i++) {
            auto resizer = new ItemResizer(directions[i], NodeSurfaceCanvas::controlGridSize);
            resizer->enablePainting();
            resizer->setVisible(false);

            // ensure corners are on top of edges
            resizer->setZValue(i > 3 ? 3 : 2);

            connect(this, &ControlItem::resizerPosChanged,
                    resizer, &ItemResizer::setPos);
            connect(this, &ControlItem::resizerSizeChanged,
                    resizer, &ItemResizer::setSize);

            connect(resizer, &ItemResizer::startDrag,
                    this, &ControlItem::resizerStartDrag);
            connect(resizer, &ItemResizer::changed,
                    this, &ControlItem::resizerChanged);

            control->selected.listen(this, [resizer]() { resizer->setVisible(true); });
            control->deselected.listen(this, [resizer]() { resizer->setVisible(false); });

            resizer->setParentItem(this);
        }
    }

    // set initial state
    setPos(control->pos());
    setSize(control->size());

    // build a state machine for hovering animation
    auto machine = new QStateMachine(this);
    auto unhoveredState = new QState(machine);
    unhoveredState->assignProperty(this, "hoverState", 0);
    machine->setInitialState(unhoveredState);

    auto hoveredState = new QState(machine);
    hoveredState->assignProperty(this, "hoverState", 1);

    auto mouseEnterTransition = unhoveredState->addTransition(this, &ControlItem::mouseEnter, hoveredState);
    auto mouseLeaveTransition = unhoveredState->addTransition(this, &ControlItem::mouseLeave, hoveredState);

    auto anim = new QPropertyAnimation(this, "hoverState");
    anim->setDuration(100);
    mouseEnterTransition->addAnimation(anim);
    mouseLeaveTransition->addAnimation(anim);

    machine->start();
}

QRectF ControlItem::boundingRect() const {
    auto br = drawBoundingRect();
    br.setHeight(br.height() + 20);
    return br;
}

QRectF ControlItem::aspectBoundingRect() const {
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

void ControlItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // draw an outline if we're connected to something
    if (!control->connections().empty()) {
        auto bounds = controlPath();
        auto activeColor = AxiomUtil::mixColor(outlineNormalColor(), outlineActiveColor(), control->isActive());
        painter->setPen(QPen(activeColor, 3));
        painter->setBrush(QBrush(activeColor));
        painter->drawPath(bounds);
    }

    //if (!control->showName()) return;

    auto br = boundingRect();
    auto pathBr = useBoundingRect();
    auto nameBr = QRectF(br.left(), pathBr.bottom() + 5, br.width(), 20);

    painter->setPen(QPen(QColor(200, 200, 200)));
    painter->setBrush(Qt::NoBrush);
    painter->drawText(nameBr, Qt::AlignHCenter | Qt::AlignTop, control->name());
}

bool ControlItem::isEditable() const {
    return !control->isSelected();
}

void ControlItem::setHoverState(float hoverState) {
    if (hoverState != _hoverState) {
        _hoverState = hoverState;
        update();
    }
}

QRectF ControlItem::drawBoundingRect() const {
    return {
        QPoint(0, 0),
        NodeSurfaceCanvas::controlRealSize(control->size())
    };
}

QColor ControlItem::outlineNormalColor() const {
    switch (control->wireType()) {
        case ConnectionWire::WireType::NUM: return CommonColors::numNormal;
        case ConnectionWire::WireType::MIDI: return CommonColors::midiNormal;
    }
    unreachable;
}

QColor ControlItem::outlineActiveColor() const {
    switch (control->wireType()) {
        case ConnectionWire::WireType::NUM: return CommonColors::numActive;
        case ConnectionWire::WireType::MIDI: return CommonColors::midiActive;
    }
    unreachable;
}

void ControlItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier) {
        event->accept();

        isConnecting = true;
        canvas->startConnecting(this);
    } else if (!isEditable() && event->button() == Qt::LeftButton) {
        event->accept();

        isMoving = true;
        mouseStartPoint = event->screenPos();
        control->startedDragging.trigger();
    } else {
        control->surface()->grid().deselectAll();
        event->ignore();
    }
}

void ControlItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (isConnecting) {
        event->accept();

        canvas->updateConnecting(event->scenePos());
    } else if (isMoving) {
        event->accept();

        auto mouseDelta = event->screenPos() - mouseStartPoint;
        control->draggedTo.trigger(QPoint(
            qRound((float) mouseDelta.x() / NodeSurfaceCanvas::controlGridSize.width()),
            qRound((float) mouseDelta.y() / NodeSurfaceCanvas::controlGridSize.height())
        ));
    } else {
        event->ignore();
    }
}

void ControlItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (isConnecting) {
        event->accept();

        isConnecting = false;
        canvas->endConnecting(event->scenePos());
    } else if (isMoving) {
        event->accept();

        isMoving = false;
        control->finishedDragging.trigger();
    } else {
        event->ignore();
    }
}

void ControlItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    control->setIsActive(false);
    emit mouseLeave();
    control->select(true);
    mousePressEvent(event);
}

void ControlItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->setIsActive(true);
    emit mouseEnter();
}

void ControlItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!isEditable()) return;

    control->setIsActive(false);
    emit mouseLeave();
}

void ControlItem::setPos(QPoint newPos) {
    auto realPos = NodeSurfaceCanvas::controlRealPos(newPos);
    QGraphicsItem::setPos(realPos.x(), realPos.y());
    emit resizerPosChanged(realPos);
}

void ControlItem::setSize(QSize newSize) {
    emit resizerSizeChanged(NodeSurfaceCanvas::controlRealSize(newSize));
}

void ControlItem::updateSelected(bool selected) {
    if (selected) {
        setCursor(Qt::SizeAllCursor);
    } else {
        unsetCursor();
    }
}

void ControlItem::remove() {
    scene()->removeItem(this);
}

void ControlItem::resizerChanged(QPointF topLeft, QPointF bottomRight) {
    control->setCorners(QPoint(
        qRound(topLeft.x() / NodeSurfaceCanvas::controlGridSize.width()),
        qRound(topLeft.y() / NodeSurfaceCanvas::controlGridSize.height())
    ), QPoint(
        qRound(bottomRight.x() / NodeSurfaceCanvas::controlGridSize.width()),
        qRound(bottomRight.y() / NodeSurfaceCanvas::controlGridSize.height())
    ));
}

void ControlItem::resizerStartDrag() {
    control->select(true);
}

void ControlItem::triggerGeometryChange() {
    prepareGeometryChange();
}

void ControlItem::triggerUpdate() {
    update();
}
