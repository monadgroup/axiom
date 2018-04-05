#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>
#include "compiler/runtime/ValueOperator.h"

namespace AxiomGui {

    class SliderPainter {
    public:

        void paint(QPainter *painter, const QRectF &boundingRect, float hoverState, MaximRuntime::NumValue cv, bool vertical, const QColor &baseColor, const QColor &activeColor);

        void shape(QPainterPath &path, const QRectF &boundingRect, bool vertical) const;

        QRectF getBounds(const QRectF &boundingRect, bool vertical) const;

    };

}

