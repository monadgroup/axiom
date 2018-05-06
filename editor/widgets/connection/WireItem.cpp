#include "WireItem.h"

#include <QtWidgets/QGraphicsScene>
#include <editor/widgets/CommonColors.h>

// FIXME: add this file to git :P
#include "editor/model/connection/ConnectionWire.h"
#include "editor/widgets/schematic/NodeSurfaceCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

WireItem::WireItem(QObject *parent, AxiomModel::ConnectionWire *wire) : QObject(parent), wire(wire) {
    connect(wire, &ConnectionWire::routeChanged,
            this, &WireItem::updateRoute);
    connect(wire, &ConnectionWire::subPosChanged,
            this, &WireItem::updateRoute);
    connect(wire, &ConnectionWire::activeChanged,
            this, &WireItem::setActive);
    connect(wire, &ConnectionWire::removed,
            this, &WireItem::remove);

    setBrush(Qt::NoBrush);

    updateRoute();
    setActive(wire->active());
}

void WireItem::updateRoute() {
    QPainterPath path;
    auto route = wire->route();

    auto firstPos = NodeSurfaceCanvas::controlRealPos(wire->sinkA->subPos());
    auto lastPos = NodeSurfaceCanvas::controlRealPos(wire->sinkB->subPos());

    // todo: find spaces where several wires are, and make them look nice
    // e.g. - if they're going in the same direction, separate them out a bit
    //      - if they're crossing, ensure the z-index of the one crossing is either in front of
    //        or behind all the others

    if (route.size() <= 2 && firstPos.x() != lastPos.x() && firstPos.y() != lastPos.y()) {
        path.moveTo(firstPos);
        path.lineTo(QPointF(lastPos.x(), firstPos.y()));
        path.lineTo(lastPos);
    } else if (route.size() >= 2) {
        auto halfNodeSize = QPointF(NodeSurfaceCanvas::nodeGridSize.width() / 2,
                                    NodeSurfaceCanvas::nodeGridSize.height() / 2);

        for (size_t i = 0; i < route.size(); i++) {
            QPointF routePos = NodeSurfaceCanvas::nodeRealPos(route[i]) + halfNodeSize;

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
    auto normalColor =
        wire->type == ConnectionSink::Type::NUMBER ? CommonColors::numNormal : CommonColors::midiNormal;
    auto activeColor =
        wire->type == ConnectionSink::Type::NUMBER ? CommonColors::numActive : CommonColors::midiActive;

    QPen pen(active ? activeColor : normalColor, 2);
    pen.setJoinStyle(Qt::MiterJoin);
    setPen(pen);

    setZValue(active ? NodeSurfaceCanvas::activeWireZVal : NodeSurfaceCanvas::wireZVal);
}

void WireItem::remove() {
    scene()->removeItem(this);
}
