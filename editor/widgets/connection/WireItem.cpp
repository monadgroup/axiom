#include "WireItem.h"

#include <QtWidgets/QGraphicsScene>

#include "editor/model/connection/ConnectionWire.h"
#include "../schematic/SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

WireItem::WireItem(AxiomModel::ConnectionWire *wire) : wire(wire) {
    connect(wire, &ConnectionWire::routeChanged,
            this, &WireItem::updateRoute);
    connect(wire, &ConnectionWire::activeChanged,
            this, &WireItem::setActive);
    connect(wire, &ConnectionWire::removed,
            this, &WireItem::remove);

    setBrush(Qt::NoBrush);

    updateRoute(wire->route());
    setActive(wire->active());
}

void WireItem::updateRoute(const std::deque<QPoint> &route) {
    QPainterPath path;

    auto firstPos = SchematicCanvas::controlRealPos(wire->sinkA->subPos());
    auto lastPos = SchematicCanvas::controlRealPos(wire->sinkB->subPos());

    if (route.size() == 2 && firstPos.x() != lastPos.x() && firstPos.y() != lastPos.y()) {
        path.moveTo(firstPos);
        path.lineTo(QPointF(lastPos.x(), firstPos.y()));
        path.lineTo(lastPos);
    } else if (route.size() >= 2) {
        auto halfNodeSize = QPointF(SchematicCanvas::nodeGridSize.width() / 2,
                                    SchematicCanvas::nodeGridSize.height() / 2);

        for (auto i = 0; i < route.size(); i++) {
            QPointF routePos = SchematicCanvas::nodeRealPos(route[i]) + halfNodeSize;

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

void WireItem::setActive(bool active) {
    QPen pen(active ? QColor(52, 152, 219) : QColor(141, 141, 141), 2);
    pen.setJoinStyle(Qt::MiterJoin);
    setPen(pen);

    setZValue(active ? SchematicCanvas::activeWireZVal : SchematicCanvas::wireZVal);
}

void WireItem::remove() {
    scene()->removeItem(this);
}
