#include "MidiControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMenu>

#include "../surface/NodeSurfaceCanvas.h"
#include "../surface/NodeSurfacePanel.h"
#include "editor/model/Project.h"
#include "editor/model/objects/MidiControl.h"

using namespace AxiomGui;
using namespace AxiomModel;

MidiControlItem::MidiControlItem(AxiomModel::MidiControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _plugImage(":/icons/midi-plug.png") {
    control->valueChanged.connectTo(this, &MidiControlItem::triggerUpdate);
    control->connections().events().itemAdded().connectTo(this, &MidiControlItem::triggerUpdate);
    control->connections().events().itemRemoved().connectTo(this, &MidiControlItem::triggerUpdate);
}

void MidiControlItem::paintControl(QPainter *painter) {
    plugPainter.paint(painter, aspectBoundingRect(), hoverState(), std::optional<AxiomModel::NumValue>(), Qt::black,
                      _plugImage);
}

QRectF MidiControlItem::useBoundingRect() const {
    return plugPainter.getBounds(aspectBoundingRect());
}

QPainterPath MidiControlItem::controlPath() const {
    QPainterPath path;
    plugPainter.shape(path, aspectBoundingRect());
    return path;
}

void MidiControlItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    QMenu menu(canvas->panel);
    buildMenuStart(menu);

    // todo: display mode

    menu.addSeparator();
    buildMenuEnd(menu);

    menu.exec(event->screenPos());
}
