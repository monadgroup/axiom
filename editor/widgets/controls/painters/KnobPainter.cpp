#include "KnobPainter.h"

#define _USE_MATH_DEFINES

#include <math.h>

#include "editor/model/objects/NumControl.h"
#include "editor/util.h"

using namespace AxiomGui;

void KnobPainter::paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState, AxiomModel::NumValue cv,
                        const QColor &baseColor, const QColor &activeColor) {
    auto aspectWidth = aspectBoundingRect.width();
    auto scaledThickness = (0.06f + 0.04f * hoverState) * aspectWidth;
    auto outerBr = getBounds(aspectBoundingRect);
    auto ringBr = outerBr.marginsRemoved(
        QMarginsF(scaledThickness / 2, scaledThickness / 2, scaledThickness / 2, scaledThickness / 2));

    auto startAngle = 240 * 16;
    auto completeAngle = -300 * 16;

    auto minVal = std::min(cv.left, cv.right);
    auto maxVal = std::max(cv.left, cv.right);

    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, hoverState);
    auto darkerCurrent = currentColor.darker();

    // draw background
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(40, 40, 40)));
    painter->drawEllipse(outerBr);

    // draw markers
    auto scaledMarkerThickness = 0.02f * aspectWidth;
    auto centerP = outerBr.center();

    const auto markerCount = 8;
    auto activeMarkerPen = QPen(QColor(0, 0, 0), scaledMarkerThickness);
    for (auto i = 0; i <= markerCount; i++) {
        auto markerVal = (float) i / markerCount;
        if (markerVal < minVal || (markerVal == 1 && minVal == 1)) {
            activeMarkerPen.setColor(currentColor);
        } else if (markerVal < maxVal || (markerVal == 1 && maxVal == 1)) {
            activeMarkerPen.setColor(darkerCurrent);
        } else {
            activeMarkerPen.setColor(QColor(0, 0, 0));
        }
        painter->setPen(activeMarkerPen);

        auto markerAngle = startAngle / 2880. * M_PI + markerVal * completeAngle / 2880. * M_PI;
        auto markerP = centerP + QPointF(outerBr.width() / 2 * std::cos(markerAngle),
                                         -outerBr.height() / 2 * std::sin(markerAngle));
        painter->drawLine((centerP + 2 * markerP) / 3, (centerP + 10 * markerP) / 11);
    }

    // draw background ring
    auto pen = QPen(QColor(0, 0, 0), scaledThickness);
    pen.setCapStyle(Qt::FlatCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawArc(ringBr, startAngle + completeAngle * maxVal, completeAngle - completeAngle * maxVal);

    // draw max ring
    pen.setColor(darkerCurrent);
    painter->setPen(pen);
    painter->drawArc(ringBr, startAngle + completeAngle * minVal, completeAngle * maxVal - completeAngle * minVal);

    // draw min ring
    pen.setColor(currentColor);
    painter->setPen(pen);
    painter->drawArc(ringBr, startAngle, completeAngle * minVal);
}

void KnobPainter::shape(QPainterPath &path, const QRectF &aspectBoundingRect) const {
    path.addEllipse(getBounds(aspectBoundingRect));
}

QRectF KnobPainter::getBounds(const QRectF &aspectBoundingRect) const {
    auto scaledMargin = 0.12f * aspectBoundingRect.width();
    return aspectBoundingRect.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}
