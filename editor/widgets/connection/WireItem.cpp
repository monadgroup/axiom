#include "WireItem.h"

#include <QtWidgets/QGraphicsScene>

#include "editor/model/connection/ConnectionWire.h"
#include "../schematic/SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

WireItem::WireItem(AxiomModel::ConnectionWire *wire) {
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

    if (!route.empty()) {
        auto halfGridSize = QPointF(SchematicCanvas::nodeGridSize.width() / 2, SchematicCanvas::nodeGridSize.height() / 2);

        path.moveTo(SchematicCanvas::nodeRealPos(route[0]) + halfGridSize);
        for (auto i = 1; i < route.size(); i++) {
            path.lineTo(SchematicCanvas::nodeRealPos(route[i]) + halfGridSize);
        }
    }

    setPath(path);
}

void WireItem::setActive(bool active) {
    QPen pen(QColor(141, 141, 141), 2);
    pen.setJoinStyle(Qt::MiterJoin);
    if (active) pen.setColor(QColor(52, 152, 219));
    setPen(pen);
}

void WireItem::remove() {
    scene()->removeItem(this);
}
