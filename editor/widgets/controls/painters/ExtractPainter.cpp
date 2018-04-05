#include "ExtractPainter.h"

#include <math.h>
#include <iostream>
#include <bitset>

#include "editor/util.h"

using namespace AxiomGui;

void ExtractPainter::paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState,
                           AxiomModel::ExtractConnectionSink::ActiveSlotFlags activeFlags, const QColor &baseColor,
                           const QColor &activeColor) {
    auto scaledBorder = 0.06f * aspectBoundingRect.width();
    auto externBr = getBounds(aspectBoundingRect);

    auto marginF = QMarginsF(scaledBorder / 2, scaledBorder / 2, scaledBorder / 2, scaledBorder / 2);
    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, hoverState);

    // draw background
    painter->setPen(QPen(QColor(0, 0, 0), scaledBorder));
    painter->setBrush(QBrush(AxiomUtil::mixColor(QColor(30, 30, 30), QColor(45, 45, 45), hoverState)));
    painter->drawEllipse(externBr.marginsRemoved(marginF));

    // draw markers
    auto scaledMarkerThickness = 0.02f * aspectBoundingRect.width();
    auto centerP = aspectBoundingRect.center();

    const auto markerCount = sizeof(activeFlags) * CHAR_BIT;
    auto activeMarkerPen = QPen(QColor(0, 0, 0), scaledMarkerThickness);
    for (AxiomModel::ExtractConnectionSink::ActiveSlotFlags i = 0; i < markerCount; i++) {
        if (activeFlags & (1u << i)) {
            activeMarkerPen.setColor(currentColor);
        } else {
            activeMarkerPen.setColor(QColor(0, 0, 0));
        }
        painter->setPen(activeMarkerPen);

        auto markerAngle = i / (float) markerCount * 2 * M_PI;
        auto markerP = centerP + QPointF(
            externBr.width() / 2 * std::cos(markerAngle),
            externBr.height() / 2 * std::sin(markerAngle)
        );
        painter->drawLine((centerP + 2 * markerP) / 3, (centerP + 10 * markerP) / 11);
    }
}

void ExtractPainter::shape(QPainterPath &path, const QRectF &aspectBoundingRect) const {
    path.addEllipse(getBounds(aspectBoundingRect));
}

QRectF ExtractPainter::getBounds(const QRectF &aspectBoundingRect) const {
    auto scaledMargin = 0.1f * aspectBoundingRect.width();
    return aspectBoundingRect.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}
