#include "MidiControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>
#include <QtWidgets/QMenu>

#include "editor/model/Project.h"
#include "editor/model/objects/MidiControl.h"
#include "../../util.h"

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

    // todo
    /*QMenu menu;
    auto clearAction = menu.addAction(tr("C&lear Connections"));

    auto modeMenu = menu.addMenu(tr("&Display as..."));
    for (const auto &modePair : modes) {
        auto action = modeMenu->addAction(modePair.first);
        action->setCheckable(true);
        action->setChecked(control->mode() == modePair.second);

        connect(action, &QAction::triggered,
                [this, modePair]() {
                    control->setMode(modePair.second);
                });
    }

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
