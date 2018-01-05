#include "SchematicView.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>
#include <QtGui/QResizeEvent>
#include <memory>

#include "src/model/node/CustomNode.h"
#include "src/model/schematic/Schematic.h"
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
    newNode->setSize(QSize(2, 1));

    auto targetPos = QPoint(
            qRound((float) contextPos.x() / SchematicCanvas::gridSize.width()) - newNode->size().width() / 2,
            qRound((float) contextPos.y() / SchematicCanvas::gridSize.height()) - newNode->size().height() / 2
    );
    newNode->setPos(schematic->grid.findNearestAvailable(targetPos, newNode->size()));
    schematic->addItem(std::move(newNode));
}

void SchematicView::resizeEvent(QResizeEvent *event) {
    scene()->setSceneRect(0, 0, event->size().width(), event->size().height());
}

void SchematicView::contextMenuEvent(QContextMenuEvent *event) {
    contextMenu->exec(event->globalPos());
    event->accept();
}
