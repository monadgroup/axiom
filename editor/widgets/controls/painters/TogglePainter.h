#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>
#include "compiler/runtime/ValueOperator.h"

namespace AxiomGui {

    class TogglePainter {
    public:

        void paint(QPainter *painter, const QRectF &boundingRect, float hoverState, MaximRuntime::NumValue cv);

        void shape(QPainterPath &path, const QRectF &boundingRect) const;

        QRectF getBounds(const QRectF &boundingRect) const;

    };

}


