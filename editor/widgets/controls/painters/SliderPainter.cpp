#include "SliderPainter.h"

#include "../../CommonColors.h"
#include "editor/util.h"

using namespace AxiomGui;

static QPointF flip(QPointF a, bool yes) {
    if (yes) {
        return {a.y(), a.x()};
    }
    return a;
}

static QSizeF flip(QSizeF a, bool yes) {
    if (yes) {
        return {a.height(), a.width()};
    }
    return a;
}

static QRectF flip(QRectF a, bool yes) {
    if (yes) {
        return {flip(a.topLeft(), yes), flip(a.size(), yes)};
    }
    return a;
}

void SliderPainter::paint(QPainter *painter, const QRectF &boundingRect, float hoverState,
                          MaximRuntime::NumValue cv, bool vertical, const QColor &baseColor, const QColor &activeColor) {
    auto br = flip(getBounds(boundingRect, vertical), vertical);
    auto scaledThickness = (0.12f + 0.08f * hoverState) * br.height();

    auto minVal = std::min(cv.left, cv.right);
    auto maxVal = std::max(cv.left, cv.right);

    auto currentColor = AxiomUtil::mixColor(baseColor, activeColor, hoverState);
    auto darkerCurrent = currentColor.darker();

    // draw background
    painter->setPen(Qt::NoPen);
    /*if (!control->sink()->connections().empty()) {
        auto activeBorderThickness = 0.04 * br.height();
        painter->setBrush(QBrush(
            AxiomUtil::mixColor(CommonColors::numWireNormal, CommonColors::numWireActive, control->sink()->active())));
        painter->drawRect(flip(
            br.marginsAdded(QMarginsF(activeBorderThickness, activeBorderThickness, activeBorderThickness,
                                      activeBorderThickness)),
            vertical
        ));
    }*/

    painter->setBrush(QBrush(QColor(30, 30, 30)));
    painter->drawRect(flip(br, vertical));

    // draw markers
    const auto markerCount = 12;
    auto activeMarkerPen = QPen(QColor(0, 0, 0), 1);
    for (auto i = 0; i <= markerCount; i++) {
        auto markerVal = (float) i / markerCount;
        if (markerVal < minVal || (markerVal == 1 && minVal == 1)) {
            activeMarkerPen.setColor(currentColor);
        } else if (markerVal < maxVal || (markerVal == 1 && maxVal == 1)) {
            activeMarkerPen.setColor(currentColor);
        } else {
            activeMarkerPen.setColor(QColor(0, 0, 0));
        }
        painter->setPen(activeMarkerPen);

        auto markerX = br.left() + br.width() * (vertical ? 1 - markerVal : markerVal);

        auto shiftAmt = 2.5;
        if (i % 2 == 0) shiftAmt = 2;
        if (i == 0 || i == markerCount || i == markerCount / 2) shiftAmt = 1.5;
        painter->drawLine(
            flip(QPointF(markerX, br.y() + 1), vertical),
            flip(QPointF(markerX, br.y() + br.height() / shiftAmt), vertical)
        );
    }

    auto minX = br.left() + br.width() * (vertical ? 1 - minVal : minVal);
    auto maxX = br.left() + br.width() * (vertical ? 1 - maxVal : maxVal);

    auto posY = br.y() + scaledThickness / 2;
    auto leftPos = QPointF(vertical ? br.right() : br.left(), posY);
    auto rightPos = QPointF(vertical ? br.left() : br.right(), posY);
    auto minPos = QPointF(minX, posY);
    auto maxPos = QPointF(maxX, posY);

    // draw background bar
    auto pen = QPen(QColor(0, 0, 0), scaledThickness);
    pen.setCapStyle(Qt::FlatCap);
    painter->setPen(pen);
    painter->drawLine(
        flip(maxPos, vertical),
        flip(rightPos, vertical)
    );

    // draw max bar
    pen.setColor(darkerCurrent);
    painter->setPen(pen);
    painter->drawLine(
        flip(minPos, vertical),
        flip(maxPos, vertical)
    );

    // draw min bar
    pen.setColor(currentColor);
    painter->setPen(pen);
    painter->drawLine(
        flip(leftPos, vertical),
        flip(minPos, vertical)
    );
}

void SliderPainter::shape(QPainterPath &path, const QRectF &boundingRect, bool vertical) const {
    path.addRect(getBounds(boundingRect, vertical));
}

QRectF SliderPainter::getBounds(const QRectF &boundingRect, bool vertical) const {
    auto br = flip(boundingRect, vertical);
    auto scaledMargin = 0.1f * br.height();
    auto barHeight = br.height() / 2;
    auto barY = br.center().y() - barHeight / 2;
    return flip(QRectF(QPointF(br.x() + scaledMargin,
                               barY),
                       QSizeF(br.width() - scaledMargin * 2,
                              barHeight)), vertical);
}
