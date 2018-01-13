#include "KnobControl.h"

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <math.h>

#include "src/model/control/NodeValueControl.h"
#include "../node/NodeItem.h"
#include "../schematic/SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

KnobControl::KnobControl(NodeValueControl *control, NodeItem *parent) : control(control), parent(parent) {
    connect(control, &NodeValueControl::posChanged,
            this, &KnobControl::setPos);
    connect(control, &NodeValueControl::beforeSizeChanged,
            this, &KnobControl::triggerGeometryChange);
    connect(control, &NodeValueControl::sizeChanged,
            this, &KnobControl::setSize);
    connect(control, &NodeValueControl::removed,
            this, &KnobControl::remove);

    connect(control, &NodeValueControl::valueChanged,
            this, &KnobControl::valueChanged);

    setPos(control->pos());
    setSize(control->size());
}

QRectF KnobControl::boundingRect() const {
    return {
            QPoint(0, 0),
            SchematicCanvas::controlRealSize(control->size())
    };
}

QRectF KnobControl::aspectBoundingRect() const {
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

void KnobControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto br = aspectBoundingRect();
    auto scaledMargin = 0.1f * br.width();
    auto scaledM = QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin);
    auto outerBr = br.marginsRemoved(scaledM);
    auto ringBr = outerBr.marginsRemoved(scaledM);

    auto startAngle = 240 * 16;
    auto completeAngle = -300 * 16;

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(15, 15, 15)));
    painter->drawEllipse(outerBr);

    painter->setPen(QPen(QColor(0, 0, 0), 0.06f * br.width()));
    painter->setBrush(Qt::NoBrush);

    painter->drawArc(ringBr, startAngle + completeAngle * control->value(), completeAngle - completeAngle * control->value());

    painter->setPen(QPen(QColor(52, 152, 219), 0.06f * br.width()));
    painter->drawArc(ringBr, startAngle, completeAngle * control->value());
}

void KnobControl::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;

    isDragging = true;
    beforeDragVal = control->value();
    dragMouseStart = event->pos();
}

void KnobControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->pos() - dragMouseStart;
    auto accuracy = 100 + (float)mouseDelta.x() * 2;
    control->setValue(beforeDragVal - (float)mouseDelta.y() / accuracy);
}

void KnobControl::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
}

void KnobControl::setPos(QPoint newPos) {
    auto realPos = SchematicCanvas::controlRealPos(newPos);
    QGraphicsItem::setPos(realPos.x(), realPos.y());
}

void KnobControl::setSize(QSize newSize) {
}

void KnobControl::remove() {
    parent->removeFromGroup(this);
}

void KnobControl::triggerGeometryChange() {
    prepareGeometryChange();
}

void KnobControl::valueChanged(float newVal) {
    update();
}
