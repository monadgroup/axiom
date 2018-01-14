#include "SchematicView.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>
#include <QtGui/QResizeEvent>
#include <memory>

#include "src/model/node/CustomNode.h"
#include "src/model/schematic/Schematic.h"
#include "src/model/control/NodeValueControl.h"
#include "SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

SchematicView::SchematicView(Schematic *schematic, QWidget *parent)
        : QGraphicsView(new SchematicCanvas(schematic), parent), schematic(schematic) {
    scene()->setSceneRect(0, 0, width() * 2, height() * 2);

    // set properties
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // build menu
    contextMenu = new QMenu(this);

    auto contextSearch = new QLineEdit();
    contextSearch->setPlaceholderText("Search modules...");
    auto widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(contextSearch);

    contextMenu->addAction(widgetAction);
    contextMenu->addSeparator();

    auto newNodeAction = new QAction(tr("New Node"));
    connect(newNodeAction, &QAction::triggered,
            this, &SchematicView::newNode);
    contextMenu->addAction(newNodeAction);

    contextMenu->addSeparator();
    contextMenu->addAction(new QAction(tr("LFO")));
    contextMenu->addAction(new QAction(tr("Something else")));
}

void SchematicView::newNode() {
    auto newNode = std::make_unique<AxiomModel::CustomNode>(schematic);
    auto contextPos = mapFromGlobal(contextMenu->pos());
    newNode->setSize(QSize(5, 5));

    auto targetPos = QPoint(
            qRound((float) contextPos.x() / SchematicCanvas::nodeGridSize.width()) - newNode->size().width() / 2,
            qRound((float) contextPos.y() / SchematicCanvas::nodeGridSize.height()) - newNode->size().height() / 2
    );
    newNode->setPos(schematic->grid.findNearestAvailable(targetPos, newNode->size()));

    auto newControl = std::make_unique<AxiomModel::NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::KNOB,
            NodeValueControl::Channel::BOTH
    );
    newControl->setCorners(QPoint(0, 0), QPoint(2, 2));
    newControl->setValue(0.2);
    newNode->surface.addItem(std::move(newControl));

    auto newControl2 = std::make_unique<AxiomModel::NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::KNOB,
            NodeValueControl::Channel::BOTH
    );
    newControl2->setCorners(QPoint(2, 0), QPoint(4, 2));
    newControl2->setValue(0.4);
    newNode->surface.addItem(std::move(newControl2));

    auto newControl3 = std::make_unique<AxiomModel::NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::KNOB,
            NodeValueControl::Channel::BOTH
    );
    newControl3->setCorners(QPoint(0, 2), QPoint(2, 4));
    newNode->surface.addItem(std::move(newControl3));

    schematic->addItem(std::move(newNode));
}

void SchematicView::resizeEvent(QResizeEvent *event) {
    scene()->setSceneRect(0, 0, event->size().width(), event->size().height());
}

void SchematicView::contextMenuEvent(QContextMenuEvent *event) {
    contextMenu->exec(event->globalPos());
    event->accept();
}
