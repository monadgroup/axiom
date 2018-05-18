#include "MidiControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMenu>

#include "editor/model/Project.h"
#include "editor/model/objects/MidiControl.h"

using namespace AxiomGui;
using namespace AxiomModel;

/*static std::vector<std::pair<QString, NodeMidiControl::Mode>> modes = {
    std::make_pair("&Plug", NodeMidiControl::Mode::PLUG),
    std::make_pair("&Piano", NodeMidiControl::Mode::PIANO)
};*/

MidiControlItem::MidiControlItem(AxiomModel::MidiControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
    control->valueChanged.connect(this, &MidiControlItem::triggerUpdate);
    control->connections().itemAdded.connect(this, &MidiControlItem::triggerUpdate);
    control->connections().itemRemoved.connect(this, &MidiControlItem::triggerUpdate);
}

void MidiControlItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    ControlItem::paint(painter, option, widget);
    plugPainter.paint(painter, aspectBoundingRect(), hoverState());
}

QPainterPath MidiControlItem::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();
    return controlPath();
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

    QMenu menu;
    buildMenuStart(menu);

    // todo: display mode

    menu.addSeparator();
    buildMenuEnd(menu);

    menu.exec(event->screenPos());
}
