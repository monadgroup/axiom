#include "ExtractControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMenu>

#include "../../util.h"
#include "../surface/NodeSurfaceCanvas.h"
#include "../surface/NodeSurfacePanel.h"
#include "editor/model/Project.h"

using namespace AxiomGui;
using namespace AxiomModel;

static QString getPlugImagePath(ExtractControl *control) {
    switch (control->wireType()) {
    case ConnectionWire::WireType::NUM:
        return ":/icons/num-plug.png";
    case ConnectionWire::WireType::MIDI:
        return ":/icons/midi-plug.png";
    }
    unreachable;
}

ExtractControlItem::ExtractControlItem(ExtractControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _plugImage(getPlugImagePath(control)) {
    control->activeSlotsChanged.connectTo(this, &ExtractControlItem::triggerUpdate);
    control->connections().events().itemAdded().connectTo(this, &ExtractControlItem::triggerUpdate);
    control->connections().events().itemRemoved().connectTo(this, &ExtractControlItem::triggerUpdate);
}

void ExtractControlItem::paintControl(QPainter *painter) {
    extractPainter.paint(painter, aspectBoundingRect(), hoverState(), control->activeSlots(), outlineNormalColor(),
                         outlineActiveColor(), _plugImage);
}

QRectF ExtractControlItem::useBoundingRect() const {
    return extractPainter.getBounds(aspectBoundingRect());
}

QPainterPath ExtractControlItem::controlPath() const {
    QPainterPath path;
    extractPainter.shape(path, drawBoundingRect());
    return path;
}

void ExtractControlItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    event->accept();

    QMenu menu(canvas->panel);
    buildMenuStart(menu);
    menu.addSeparator();
    buildMenuEnd(menu);

    menu.exec(event->screenPos());
}
