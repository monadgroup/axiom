#pragma once

#include <optional>
#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include "editor/model/Value.h"

namespace AxiomGui {

    class PlugPainter {
    public:

        void paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState,
                   std::optional<AxiomModel::NumValue> val, const QColor &valBaseColor);

        void shape(QPainterPath &path, const QRectF &aspectBoundingRect) const;

        QRectF getBounds(const QRectF &aspectBoundingRect) const;

    };

}
