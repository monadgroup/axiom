#include "TogglePainter.h"

#include <cmath>

#include "../../CommonColors.h"
#include "editor/util.h"

using namespace AxiomGui;

void TogglePainter::paint(QPainter *painter, const QRectF &boundingRect, float hoverState, AxiomModel::NumValue cv) {
    auto br = getBounds(boundingRect);

    auto scaleFactor = std::hypot(br.right() - br.left(), br.bottom() - br.top());
    auto borderMargin = QMarginsF(0.02 * br.width(), 0.02 * br.height(), 0.02 * br.width(), 0.02 * br.height());

    auto baseColor = QColor(45, 45, 45);
    auto activeColor = QColor(60, 60, 60);

    auto brightness = (cv.left + cv.right) / 2;

    // draw background
    /*if (!control->sink()->connections().empty()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(AxiomUtil::mixColor(connectedColor, connectedActiveColor, control->sink()->active())));
        painter->drawRect(br.marginsAdded(borderMargin));
    }*/

    painter->setPen(QPen(QColor(0, 0, 0), 0.02 * scaleFactor));
    painter->setBrush(QBrush(AxiomUtil::mixColor(baseColor, activeColor, hoverState)));
    painter->drawRect(br.marginsRemoved(borderMargin));

    // draw light
    auto lightRadius = 0.05 * scaleFactor;
    auto lightPos = QPointF(br.left() + br.width() / 2, br.top() + br.height() / 4);

    painter->setPen(Qt::NoPen);

    auto activeAlpha = QColor(CommonColors::numActive.red(), CommonColors::numActive.green(),
                              CommonColors::numActive.blue(), (int) (brightness * 255));
    auto glowRadius = lightRadius * 2;
    QRadialGradient gradient(lightPos, glowRadius);
    gradient.setColorAt(0, activeAlpha);
    gradient.setColorAt(1, QColor(activeAlpha.red(), activeAlpha.green(),
                                  activeAlpha.blue(), 0));
    painter->setBrush(gradient);
    painter->drawEllipse(lightPos, glowRadius, glowRadius);

    painter->setBrush(QBrush(AxiomUtil::mixColor(Qt::black, CommonColors::numActive, brightness)));
    painter->drawEllipse(lightPos, lightRadius, lightRadius);
}

void TogglePainter::shape(QPainterPath &path, const QRectF &boundingRect) const {
    path.addRect(getBounds(boundingRect));
}

QRectF TogglePainter::getBounds(const QRectF &boundingRect) const {
    auto hMargin = 0.1f * boundingRect.width();
    auto vMargin = 0.1f * boundingRect.height();
    return boundingRect.marginsRemoved(QMarginsF(hMargin, vMargin, hMargin, vMargin));
}
