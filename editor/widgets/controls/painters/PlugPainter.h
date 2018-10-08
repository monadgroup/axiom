#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>
#include <optional>

#include "editor/model/Value.h"

namespace AxiomGui {

    class PlugPainter {
    public:
        void paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState,
                   std::optional<AxiomModel::NumValue> val, const QColor &valBaseColor, const QImage &image);

        void shape(QPainterPath &path, const QRectF &aspectBoundingRect) const;

        QRectF getBounds(const QRectF &aspectBoundingRect) const;
    };
}
