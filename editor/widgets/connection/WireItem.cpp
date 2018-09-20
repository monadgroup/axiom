#include "WireItem.h"

#include <QtWidgets/QGraphicsScene>
#include <editor/widgets/CommonColors.h>

#include "../surface/NodeSurfaceCanvas.h"
#include "editor/model/ConnectionWire.h"
#include "editor/model/WireGrid.h"

using namespace AxiomGui;
using namespace AxiomModel;

const float WIRE_SEPARATION = 6;

WireItem::WireItem(QObject *parent, AxiomModel::ConnectionWire *wire) : QObject(parent), wire(wire) {
    wire->routeChanged.connect(this, &WireItem::updateRoute);
    wire->activeChanged.connect(this, &WireItem::setIsActive);
    wire->removed.connect(this, &WireItem::remove);

    setBrush(Qt::NoBrush);

    wire->wireGrid()->tryFlush();
    updateRoute(wire->route(), wire->lineIndices());
    setIsActive(wire->active());
}

void WireItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto normalColor =
        wire->wireType() == ConnectionWire::WireType::NUM ? CommonColors::numNormal : CommonColors::midiNormal;
    auto activeColor =
        wire->wireType() == ConnectionWire::WireType::NUM ? CommonColors::numActive : CommonColors::midiActive;
    auto currentColor = wire->active() ? activeColor : normalColor;

    QPen pen(QColor(20, 20, 20), 4);
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);
    painter->drawPath(path());

    pen.setColor(currentColor);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawPath(path());
}

static QPointF getRealPos(size_t index, QPointF routePos, const std::deque<QPoint> &route,
                          const std::vector<AxiomModel::LineIndex> &lineIndices) {
    if (index >= 1) {
        auto &lastLineIndex = lineIndices[index - 1];
        auto separationIndex = lastLineIndex.index - (lastLineIndex.count - 1.f) / 2.f;

        auto deltaRoute = route[index - 1] - route[index];
        if (deltaRoute.x()) routePos.setY(routePos.y() + separationIndex * WIRE_SEPARATION);
        if (deltaRoute.y()) routePos.setX(routePos.x() + separationIndex * WIRE_SEPARATION);
    }
    if (index < lineIndices.size()) {
        auto &nextLineIndex = lineIndices[index];
        auto separationIndex = nextLineIndex.index - (nextLineIndex.count - 1.f) / 2.f;

        auto deltaRoute = route[index] - route[index + 1];
        if (deltaRoute.x()) routePos.setY(routePos.y() + separationIndex * WIRE_SEPARATION);
        if (deltaRoute.y()) routePos.setX(routePos.x() + separationIndex * WIRE_SEPARATION);
    }

    return routePos;
}

void WireItem::updateRoute(const std::deque<QPoint> &route, const std::vector<AxiomModel::LineIndex> &lineIndices) {
    QPainterPath path;

    auto halfNodeSize =
        QPointF(NodeSurfaceCanvas::nodeGridSize.width() / 2, NodeSurfaceCanvas::nodeGridSize.height() / 2);

    auto firstPos = getRealPos(0, NodeSurfaceCanvas::nodeRealPos(wire->startPos()), route, lineIndices);
    auto lastPos = getRealPos(route.size() - 1, NodeSurfaceCanvas::nodeRealPos(wire->endPos()), route, lineIndices);

    if (route.size() <= 2 && firstPos.x() != lastPos.x() && firstPos.y() != lastPos.y()) {
        path.moveTo(firstPos);
        path.lineTo(QPointF(lastPos.x(), firstPos.y()));
        path.lineTo(lastPos);
    } else if (route.size() >= 2) {
        for (size_t i = 0; i < route.size(); i++) {
            auto routePos = getRealPos(i, NodeSurfaceCanvas::nodeRealPos(route[i]) + halfNodeSize, route, lineIndices);

            if (i == 0) {
                routePos = firstPos;
            } else if (i == route.size() - 1) {
                routePos = lastPos;
            } else {
                if (i == 1) {
                    auto deltaRoute = route[1] - route[0];
                    if (deltaRoute.x()) routePos.setY(firstPos.y());
                    if (deltaRoute.y()) routePos.setX(firstPos.x());
                }
                if (i == route.size() - 2) {
                    auto deltaRoute = route[route.size() - 2] - route[route.size() - 1];
                    if (deltaRoute.x()) routePos.setY(lastPos.y());
                    if (deltaRoute.y()) routePos.setX(lastPos.x());
                }
            }

            if (i == 0) {
                path.moveTo(routePos);
            } else {
                path.lineTo(routePos);
            }
        }
    }

    setPath(path);
}

void WireItem::setIsActive(bool active) {
    auto normalColor =
        wire->wireType() == ConnectionWire::WireType::NUM ? CommonColors::numNormal : CommonColors::midiNormal;
    auto activeColor =
        wire->wireType() == ConnectionWire::WireType::NUM ? CommonColors::numActive : CommonColors::midiActive;

    QPen pen(active ? activeColor : normalColor, 2);
    pen.setJoinStyle(Qt::MiterJoin);
    setPen(pen);

    setZValue(active ? NodeSurfaceCanvas::activeWireZVal : NodeSurfaceCanvas::wireZVal);
}

void WireItem::remove() {
    scene()->removeItem(this);
}
