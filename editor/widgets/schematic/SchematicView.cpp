#include "SchematicView.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>
#include <QtGui/QResizeEvent>
#include <memory>

#include "editor/model/node/CustomNode.h"
#include "editor/model/schematic/Schematic.h"
#include "editor/model/control/NodeValueControl.h"
#include "SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

SchematicView::SchematicView(Schematic *schematic, QWidget *parent)
        : QGraphicsView(new SchematicCanvas(schematic), parent), schematic(schematic) {
    scene()->setSceneRect(0, 0, width() * 2, height() * 2);

    // set properties
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);

    setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);
    centerOn(0, 0);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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

void SchematicView::pan(QPointF delta) {
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    auto newCenter = QPoint(viewport()->rect().width() / 2 - delta.x(), viewport()->rect().height() / 2 - delta.y());
    centerOn(mapToScene(newCenter));
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}

void SchematicView::newNode() {
    auto contextPos = mapToScene(mapFromGlobal(contextMenu->pos()));
    auto defaultSize = QSize(5, 5);
    auto targetPos = QPoint(
            qRound((float) contextPos.x() / SchematicCanvas::nodeGridSize.width()) - defaultSize.width() / 2,
            qRound((float) contextPos.y() / SchematicCanvas::nodeGridSize.height()) - defaultSize.height() / 2
    );

    auto newNode = std::make_unique<AxiomModel::CustomNode>(schematic, targetPos, defaultSize);

    auto newControl = std::make_unique<AxiomModel::NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::BASIC,
            NodeValueControl::Channel::BOTH,
            QPoint(0, 0), QSize(2, 2)
    );
    newControl->setValue(0.2);
    newNode->surface.addItem(std::move(newControl));

    auto newControl2 = std::make_unique<AxiomModel::NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::BASIC,
            NodeValueControl::Channel::BOTH,
            QPoint(2, 0), QSize(2, 2)
    );
    newControl2->setValue(0.4);
    newNode->surface.addItem(std::move(newControl2));

    auto newControl3 = std::make_unique<AxiomModel::NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::BASIC,
            NodeValueControl::Channel::BOTH,
            QPoint(0, 2), QSize(2, 6)
    );
    newNode->surface.addItem(std::move(newControl3));

    auto newControl4 = std::make_unique<AxiomModel::NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::BASIC,
            NodeValueControl::Channel::BOTH,
            QPoint(2, 2), QSize(6, 2)
    );
    newNode->surface.addItem(std::move(newControl4));

    auto newControl5 = std::make_unique<AxiomModel::NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::TOGGLE,
            NodeValueControl::Channel::BOTH,
            QPoint(4, 4), QSize(2, 2)
    );
    newNode->surface.addItem(std::move(newControl5));

    schematic->addItem(std::move(newNode));
}

void SchematicView::contextMenuEvent(QContextMenuEvent *event) {
    contextMenu->exec(event->globalPos());
    event->accept();
}

void SchematicView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning = true;
        lastMousePos = event->pos();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }

    QGraphicsView::mousePressEvent(event);
}

void SchematicView::mouseMoveEvent(QMouseEvent *event) {
    if (isPanning) {
        pan(mapToScene(event->pos()) - mapToScene(lastMousePos));
        lastMousePos = event->pos();
    }

    QGraphicsView::mouseMoveEvent(event);
}

void SchematicView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning = false;
        QApplication::restoreOverrideCursor();
    }

    QGraphicsView::mouseReleaseEvent(event);
}
