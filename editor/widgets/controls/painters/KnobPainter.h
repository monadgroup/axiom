#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>
#include "compiler/runtime/ValueOperator.h"

namespace AxiomGui {

    class KnobPainter {
    public:

        void paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState, MaximRuntime::NumValue cv,
                   const QColor &baseColor, const QColor &activeColor);

        void shape(QPainterPath &path, const QRectF &aspectBoundingRect) const;

        QRectF getBounds(const QRectF &aspectBoundingRect) const;

    };

}
