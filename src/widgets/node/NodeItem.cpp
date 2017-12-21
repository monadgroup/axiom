#include "NodeItem.h"

#include <QtGui/QPen>
#include <QtGui/QPainter>

using namespace AxiomGui;

NodeItem::NodeItem(QGraphicsItem *parent) : QGraphicsObject(parent) {

}

QRectF NodeItem::boundingRect() const {
    return {150, 200, 100, 100};
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(QPen(QColor::fromRgb(51, 51, 51), 1));
    painter->setBrush(QBrush(QColor::fromRgb(17, 17, 17)));
    painter->drawRect(boundingRect());
}
