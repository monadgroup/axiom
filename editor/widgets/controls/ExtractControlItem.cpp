#include "ExtractControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMenu>
#include <cassert>

#include "editor/model/Project.h"
#include "editor/model/objects/ExtractControl.h"
#include "../../util.h"

using namespace AxiomGui;
using namespace AxiomModel;

ExtractControlItem::ExtractControlItem(ExtractControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control) {
    control->activeSlotsChanged.listen<ControlItem>(this, &ExtractControlItem::triggerUpdate);
    control->connections().itemAdded.listen<ControlItem>(this, &ExtractControlItem::triggerUpdate);
    control->connections().itemRemoved.listen<ControlItem>(this, &ExtractControlItem::triggerUpdate);
}

void ExtractControlItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    ControlItem::paint(painter, option, widget);

    extractPainter.paint(painter, aspectBoundingRect(), hoverState(), control->activeSlots(), outlineNormalColor(),
                         outlineActiveColor());
}

QPainterPath ExtractControlItem::shape() const {
    if (control->isSelected()) return QGraphicsItem::shape();
    return controlPath();
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

    // todo
    /*QMenu menu;
    auto clearAction = menu.addAction(tr("C&lear Connections"));
    menu.addSeparator();
    auto moveAction = menu.addAction(tr("&Move"));
    auto nameShownAction = menu.addAction(tr("Show &Name"));
    nameShownAction->setCheckable(true);
    nameShownAction->setChecked(control->showName());

    QAction *exposedAction = nullptr;
    if (control->node->parentSchematic->canExposeControl()) {
        // todo: make this checkable
        exposedAction = menu.addAction(tr("&Expose"));
    }

    auto selectedAction = menu.exec(event->screenPos());

    if (selectedAction == clearAction) {
        DO_ACTION(control->node->parentSchematic->project()->history, HistoryList::ActionType::DISCONNECT_ALL, {
            control->sink()->clearConnections();
        });
    } else if (selectedAction == moveAction) {
        control->select(true);
    } else if (selectedAction == nameShownAction) {
        auto actionType = nameShownAction->isChecked() ? HistoryList::ActionType::SHOW_CONTROL_NAME : HistoryList::ActionType::HIDE_CONTROL_NAME;
        DO_ACTION(control->node->parentSchematic->project()->history, actionType, {
            control->setShowName(nameShownAction->isChecked());
        });
    } else if (exposedAction && selectedAction == exposedAction) {
        control->node->parentSchematic->exposeControl(control);
    }*/
}
