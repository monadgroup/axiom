#include "PlugPainter.h"

#include "editor/util.h"
#include "editor/model/Value.h"

using namespace AxiomGui;

void PlugPainter::paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState, std::optional<AxiomModel::NumValue> val, const QColor &valBaseColor) {
    auto scaledBorder = 0.06f * aspectBoundingRect.width();
    auto externBr = getBounds(aspectBoundingRect);

    auto marginF = QMarginsF(scaledBorder / 2, scaledBorder / 2, scaledBorder / 2, scaledBorder / 2);

    auto baseColor = QColor(45, 45, 45);
    auto activeColor = QColor(60, 60, 60);

    painter->setPen(QPen(QColor(0, 0, 0), scaledBorder));
    if (val) {
        painter->setBrush(Qt::NoBrush);
    } else {
        painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, hoverState)));
    }

    auto ellipseBounds = externBr.marginsRemoved(marginF);
    painter->drawEllipse(ellipseBounds);

    if (!val) return;

    auto startAngle = 240 * 16;
    auto completeAngle = -300 * 16;

    auto minVal = std::min(val->left, val->right);
    auto maxVal = std::max(val->left, val->right);

    // draw max ring
    painter->setPen(QPen(valBaseColor.darker(), scaledBorder));
    painter->drawArc(ellipseBounds, startAngle + completeAngle * minVal, completeAngle * maxVal - completeAngle * minVal);

    // draw min ring
    painter->setPen(QPen(valBaseColor, scaledBorder));
    painter->drawArc(ellipseBounds, startAngle, completeAngle * minVal);

    painter->setPen(QPen(Qt::transparent, scaledBorder));
    painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, hoverState)));
    painter->drawEllipse(ellipseBounds.marginsRemoved(marginF));
}

void PlugPainter::shape(QPainterPath &path, const QRectF &aspectBoundingRect) const {
    path.addEllipse(getBounds(aspectBoundingRect));
}

QRectF PlugPainter::getBounds(const QRectF &aspectBoundingRect) const {
    auto scaledMargin = 0.12f * aspectBoundingRect.width();
    return aspectBoundingRect.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}
