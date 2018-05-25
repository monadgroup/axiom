#include "ModulePreviewView.h"

#include <QtWidgets/QGraphicsItem>
#include <QtGui/QMouseEvent>

#include "ModulePreviewCanvas.h"
#include "../node/NodeItem.h"
#include "../windows/MainWindow.h"
#include "../windows/ModulePropertiesWindow.h"
#include "editor/model/Library.h"
#include "editor/model/LibraryEntry.h"
#include "editor/model/objects/RootSurface.h"

using namespace AxiomGui;

ModulePreviewView::ModulePreviewView(MainWindow *window, AxiomModel::Library *library, AxiomModel::LibraryEntry *entry,
                                     QWidget *parent)
    : QGraphicsView(parent), window(window), library(library), entry(entry) {
    auto moduleScene = new ModulePreviewCanvas(entry->rootSurface());
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
    /*QGraphicsView::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        event->accept();
        auto drag = new QDrag(this);

        // serialize the surface and set mime data
        QByteArray serializeArray;
        QDataStream stream(&serializeArray, QIODevice::WriteOnly);
        entry->schematic().partialSerialize(stream, entry->schematic().rawItems(), QPoint(0, 0));

        auto mimeData = new QMimeData();
        mimeData->setData("application/axiom-partial-surface", serializeArray);
        drag->setMimeData(mimeData);

        drag->exec();
    }*/
}

void ModulePreviewView::mouseDoubleClickEvent(QMouseEvent *) {
    window->showSurface(nullptr, entry->rootSurface(), true);
}

void ModulePreviewView::contextMenuEvent(QContextMenuEvent *event) {
    /*event->accept();

    QMenu menu;

    auto editAction = menu.addAction(tr("&Edit"));
    auto propertiesAction = menu.addAction(tr("&Properties..."));
    menu.addSeparator();
    auto deleteAction = menu.addAction(tr("&Delete"));
    auto selectedAction = menu.exec(event->globalPos());

    if (selectedAction == editAction) {
        window->showSchematic(nullptr, &entry->schematic(), true);
    } else if (selectedAction == propertiesAction) {
        ModulePropertiesWindow propWindow(library);
        propWindow.setEnteredName(entry->name());

        QStringList currentTags;
        for (const auto &tag : entry->tags()) {
            currentTags.push_back(tag);
        }
        propWindow.setEnteredTags(currentTags);

        if (propWindow.exec() == QDialog::Accepted) {
            entry->setName(propWindow.enteredName());

            auto enteredTags = propWindow.enteredTags();
            std::set<QString> newTags(enteredTags.begin(), enteredTags.end());

            // remove old tags
            std::set<QString> oldTags(entry->tags());
            for (const auto &oldTag : oldTags) {
                if (newTags.find(oldTag) == newTags.end()) entry->removeTag(oldTag);
            }

            // add new tags
            for (const auto &newTag : newTags) {
                entry->addTag(newTag);
            }
        }
    } else if (selectedAction == deleteAction) {
        QMessageBox confirmBox(QMessageBox::Warning, "Confirm Delete", "Are you sure you want to delete this module?\n\n"
                                                                       "This operation cannot be undone.");
        confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        confirmBox.setDefaultButton(QMessageBox::Yes);

        if (confirmBox.exec() == QMessageBox::Yes) {
            entry->remove();
        }
    }*/
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

    scale(1 / currentScale, 1 / currentScale);
    centerOn(boundingRect.center());
    scale(scaleFactor, scaleFactor);
    currentScale = scaleFactor;
}
