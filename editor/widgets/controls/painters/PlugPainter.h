#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>
#include "compiler/runtime/ValueOperator.h"

namespace AxiomGui {

    class PlugPainter {
    public:

        void paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState, std::optional<MaximRuntime::NumValue> val, const QColor &valBaseColor);

        void shape(QPainterPath &path, const QRectF &aspectBoundingRect) const;

        QRectF getBounds(const QRectF &aspectBoundingRect) const;

    };

}
