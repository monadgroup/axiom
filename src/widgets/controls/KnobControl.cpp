#include "KnobControl.h"

#include <QtGui/QPainter>
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

    painter->setPen(QPen(Qt::red));
    painter->setBrush(QBrush(Qt::blue));
    painter->drawRect(br);
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
