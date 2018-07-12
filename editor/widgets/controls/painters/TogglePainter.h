#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include "editor/model/Value.h"

namespace AxiomGui {

    class TogglePainter {
    public:

        void paint(QPainter *painter, const QRectF &boundingRect, float hoverState, AxiomModel::NumValue cv);

        void shape(QPainterPath &path, const QRectF &boundingRect) const;

        QRectF getBounds(const QRectF &boundingRect) const;

    };

}


