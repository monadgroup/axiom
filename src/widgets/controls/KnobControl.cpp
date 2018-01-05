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

void KnobControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto br = boundingRect();

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
