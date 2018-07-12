#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include "editor/model/Value.h"

namespace AxiomGui {

    class KnobPainter {
    public:

        void paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState, AxiomModel::NumValue cv,
                   const QColor &baseColor, const QColor &activeColor);

        void shape(QPainterPath &path, const QRectF &aspectBoundingRect) const;

        QRectF getBounds(const QRectF &aspectBoundingRect) const;

    };

}
