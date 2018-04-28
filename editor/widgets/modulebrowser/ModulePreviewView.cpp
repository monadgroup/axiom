#include "ModulePreviewView.h"

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsItem>
#include <QtGui/QMouseEvent>
#include <QtGui/QDrag>
#include <QtCore/QMimeData>

#include "ModulePreviewCanvas.h"
#include "../node/NodeItem.h"
#include "editor/model/LibraryEntry.h"
#include "editor/model/schematic/LibrarySchematic.h"
#include "../../util.h"

using namespace AxiomGui;

ModulePreviewView::ModulePreviewView(AxiomModel::LibraryEntry *entry, QWidget *parent)
    : QGraphicsView(parent), entry(entry) {
    auto moduleScene = new ModulePreviewCanvas(&entry->schematic());
    setScene(moduleScene);
    scene()->setParent(this);

    setInteractive(false);
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setFixedSize(100, 100);
    updateScaling();
    connect(moduleScene, &ModulePreviewCanvas::contentChanged,
            this, &ModulePreviewView::updateScaling);
}

void ModulePreviewView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        auto drag = new QDrag(this);

        // serialize the surface and set mime data
        QByteArray serializeArray;
        QDataStream stream(&serializeArray, QIODevice::WriteOnly);
        entry->schematic().partialSerialize(stream, entry->schematic().rawItems(), QPoint(0, 0));

        auto mimeData = new QMimeData();
        mimeData->setData("application/axiom-partial-surface", serializeArray);
        drag->setMimeData(mimeData);

        drag->exec();
    }
}

void ModulePreviewView::updateScaling() {
    // figure out the bounding box size of the scene
    QRectF boundingRect;
    for (const auto &item : scene()->items()) {
        if (auto node = dynamic_cast<NodeItem*>(item)) {
            auto br = node->drawBoundingRect();
            br.setTopLeft(node->scenePos());
            boundingRect = boundingRect.united(br);
        }
    }
    setSceneRect(boundingRect);

    // figure out correct scaling with some padding
    auto selfWidth = width() - 30;
    auto selfHeight = height() - 30;
    auto scaleFactor = boundingRect.width() > boundingRect.height() ? selfWidth / boundingRect.width() : selfHeight / boundingRect.height();

    centerOn(boundingRect.center());
    scale(scaleFactor, scaleFactor);
}
