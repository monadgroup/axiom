#include "ModulePreviewView.h"

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsItem>
#include <iostream>

#include "ModulePreviewCanvas.h"
#include "../node/NodeItem.h"
#include "editor/model/LibraryEntry.h"
#include "editor/model/schematic/LibrarySchematic.h"
#include "../../util.h"

using namespace AxiomGui;

ModulePreviewView::ModulePreviewView(AxiomModel::LibraryEntry *entry, QWidget *parent)
    : QGraphicsView(new ModulePreviewCanvas(&entry->schematic()), parent), entry(entry) {
    scene()->setParent(this);

    setInteractive(false);
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // figure out the bounding box size of the scene
    QRectF boundingRect;
    for (const auto &item : scene()->items()) {
        if (auto node = dynamic_cast<NodeItem*>(item)) {
            boundingRect = boundingRect.united(node->drawBoundingRect());
        }
    }
    setSceneRect(boundingRect);

    setFixedSize(100, 100);

    // figure out correct scaling with some padding
    auto selfWidth = width() - 30;
    auto selfHeight = height() - 30;
    auto scaleFactor = boundingRect.width() > boundingRect.height() ? selfWidth / boundingRect.width() : selfHeight / boundingRect.height();
    scale(scaleFactor, scaleFactor);
    centerOn(boundingRect.center());
}
