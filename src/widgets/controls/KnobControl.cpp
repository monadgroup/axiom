#include "KnobControl.h"

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <cmath>

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
    auto scaledThickness = 0.06f * br.width();
    auto outerBr = br.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
    auto ringBr = outerBr.marginsRemoved(QMarginsF(scaledThickness/2, scaledThickness/2, scaledThickness/2, scaledThickness/2));

    auto startAngle = 240 * 16;
    auto completeAngle = -300 * 16;

    auto currentColor = QColor(52, 152, 219);

    // draw background
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(30, 30, 30)));
    painter->drawEllipse(outerBr);

    // draw markers
    auto scaledMarkerThickness = 0.02f * br.width();
    auto centerP = outerBr.center();

    const auto markerCount = 8;
    for (auto i = 0; i <= markerCount; i++) {
        auto markerVal = (float)i / markerCount;
        if (markerVal < control->value() || (markerVal == 1 && control->value() == 1)) {
            painter->setPen(QPen(currentColor, scaledMarkerThickness));
        } else {
            painter->setPen(QPen(QColor(0, 0, 0), scaledMarkerThickness));
        }

        auto markerAngle = startAngle / 2880. * M_PI + markerVal * completeAngle / 2880. * M_PI;
        auto markerP = centerP + QPointF(
                ringBr.width() / 2 * std::cos(markerAngle),
                -ringBr.height() / 2 * std::sin(markerAngle)
        );
        painter->drawLine((centerP + 2*markerP) / 3, markerP);
    }

    // draw background ring
    auto pen = QPen(QColor(0, 0, 0), scaledThickness);
    pen.setCapStyle(Qt::FlatCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawArc(ringBr, startAngle + completeAngle * control->value(), completeAngle - completeAngle * control->value());

    // draw filled ring
    pen.setColor(currentColor);
    painter->setPen(pen);
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
    auto accuracy = 100 + (float)std::abs(mouseDelta.x()) * 2;
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
