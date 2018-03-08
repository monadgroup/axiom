#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>

namespace AxiomGui {

    class PlugPainter {
    public:

        void paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState);

        void shape(QPainterPath &path, const QRectF &aspectBoundingRect) const;

        QRectF getBounds(const QRectF &aspectBoundingRect) const;

    };

}
