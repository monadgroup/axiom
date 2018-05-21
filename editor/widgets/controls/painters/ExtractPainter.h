#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include "editor/model/objects/ExtractControl.h"

namespace AxiomGui {

    class ExtractPainter {
    public:

        void paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState,
                   AxiomModel::ExtractControl::ActiveSlotFlags activeFlags, const QColor &baseColor,
                   const QColor &activeColor);

        void shape(QPainterPath &path, const QRectF &aspectBoundingRect) const;

        QRectF getBounds(const QRectF &aspectBoundingRect) const;

    };

}
