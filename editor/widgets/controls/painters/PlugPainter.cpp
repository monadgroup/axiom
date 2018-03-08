#include "PlugPainter.h"

#include "editor/util.h"

using namespace AxiomGui;

void PlugPainter::paint(QPainter *painter, const QRectF &aspectBoundingRect, float hoverState) {
    auto scaledBorder = 0.06f * aspectBoundingRect.width();
    auto externBr = getBounds(aspectBoundingRect);

    auto marginF = QMarginsF(scaledBorder / 2, scaledBorder / 2, scaledBorder / 2, scaledBorder / 2);

    auto baseColor = QColor(45, 45, 45);
    auto activeColor = QColor(60, 60, 60);
    //auto connectedActiveColor = CommonColors::numWireActive;
    //auto connectedColor = CommonColors::numWireNormal;

    /*if (!control->sink()->connections().empty()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(AxiomUtil::mixColor(connectedColor, connectedActiveColor, control->sink()->active())));
        painter->drawEllipse(externBr.marginsAdded(marginF));
    }*/

    painter->setPen(QPen(QColor(0, 0, 0), scaledBorder));
    painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, hoverState)));
    painter->drawEllipse(externBr.marginsRemoved(marginF));
}

void PlugPainter::shape(QPainterPath &path, const QRectF &aspectBoundingRect) const {
    path.addEllipse(getBounds(aspectBoundingRect));
}

QRectF PlugPainter::getBounds(const QRectF &aspectBoundingRect) const {
    auto scaledMargin = 0.1f * aspectBoundingRect.width();
    return aspectBoundingRect.marginsRemoved(QMarginsF(scaledMargin, scaledMargin, scaledMargin, scaledMargin));
}
