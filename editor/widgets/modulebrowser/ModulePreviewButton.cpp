#include "ModulePreviewButton.h"

#include <QtCore/QMimeData>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QDrag>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>

#include "../node/NodeItem.h"
#include "../windows/MainWindow.h"
#include "../windows/ModulePropertiesWindow.h"
#include "ModulePreviewCanvas.h"
#include "editor/model/Library.h"
#include "editor/model/LibraryEntry.h"
#include "editor/model/objects/ModuleSurface.h"
#include "editor/model/serialize/LibrarySerializer.h"
#include "editor/model/serialize/ModelObjectSerializer.h"
#include "editor/model/serialize/ProjectSerializer.h"

using namespace AxiomGui;

ModulePreviewButton::ModulePreviewButton(MainWindow *window, AxiomModel::Library *library,
                                         AxiomModel::LibraryEntry *entry, QWidget *parent)
    : QFrame(parent), window(window), library(library), _entry(entry) {
    setFixedWidth(110);

    auto mainLayout = new QGridLayout(this);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setRowStretch(0, 1);

    image = new QLabel(this);
    image->setObjectName("image");
    mainLayout->addWidget(image, 0, 0);
    image->setMargin(0);
    image->setContentsMargins(0, 0, 0, 0);

    label = new QLabel(this);
    label->setObjectName("label");
    mainLayout->addWidget(label, 1, 0);
    label->setMargin(0);
    label->setContentsMargins(5, 0, 0, 0);

    setLayout(mainLayout);

    entry->nameChanged.connectTo(this, &ModulePreviewButton::setName);
    setName(entry->name());

    library->activeTagChanged.connectTo(this, &ModulePreviewButton::updateIsVisible);
    library->activeSearchChanged.connectTo(this, &ModulePreviewButton::updateIsVisible);

    entry->root()->history().stackChanged.connectTo(this, &ModulePreviewButton::updateImage);

    updateImage();
    updateIsVisible();
}

void ModulePreviewButton::mousePressEvent(QMouseEvent *event) {
    QFrame::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        event->accept();
        auto drag = new QDrag(this);

        // serialize the surface and set mime data
        auto centerPos = AxiomModel::GridSurface::findCenter(_entry->rootSurface()->grid().items().sequence());
        QByteArray serializeArray;
        QDataStream stream(&serializeArray, QIODevice::WriteOnly);
        stream << centerPos;
        AxiomModel::ModelObjectSerializer::serializeChunk(
            stream, _entry->rootSurface()->uuid(),
            AxiomModel::findDependents(
                AxiomCommon::dynamicCast<AxiomModel::ModelObject *>(_entry->root()->pool().sequence().sequence()),
                _entry->rootSurface()->uuid(), false));

        auto mimeData = new QMimeData();
        mimeData->setData("application/axiom-partial-surface", serializeArray);
        drag->setMimeData(mimeData);

        drag->exec();
    }
}

void ModulePreviewButton::mouseDoubleClickEvent(QMouseEvent *event) {
    window->showSurface(nullptr, _entry->rootSurface(), false, false);
}

void ModulePreviewButton::contextMenuEvent(QContextMenuEvent *event) {
    event->accept();

    QMenu menu(this);
    auto editAction = menu.addAction("&Edit");
    auto propertiesAction = menu.addAction("&Properties...");
    menu.addSeparator();
    auto exportAction = menu.addAction("E&xport...");
    menu.addSeparator();
    auto deleteAction = menu.addAction("&Delete");

    auto selectedAction = menu.exec(event->globalPos());
    if (selectedAction == editAction) {
        window->showSurface(nullptr, _entry->rootSurface(), false, false);
    } else if (selectedAction == propertiesAction) {
        ModulePropertiesWindow propWindow(library);
        propWindow.setEnteredName(_entry->name());

        QStringList currentTags;
        for (const auto &tag : _entry->tags()) {
            currentTags.push_back(tag);
        }
        propWindow.setEnteredTags(currentTags);

        if (propWindow.exec() == QDialog::Accepted) {
            _entry->setName(propWindow.enteredName());

            auto enteredTags = propWindow.enteredTags();
            std::set<QString> newTags(enteredTags.begin(), enteredTags.end());

            // remove old tags
            std::set<QString> oldTags(_entry->tags());
            for (const auto &oldTag : oldTags) {
                if (newTags.find(oldTag) == newTags.end()) _entry->removeTag(oldTag);
            }

            // add new tags
            for (const auto &newTag : newTags) {
                _entry->addTag(newTag);
            }
        }
    } else if (selectedAction == exportAction) {
        auto selectedFile = QFileDialog::getSaveFileName(this, "Export Module", QString(),
                                                         tr("Axiom Library Files (*.axl);;All Files (*.*)"));
        if (selectedFile.isNull()) return;

        QFile file(selectedFile);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox msgBox(QMessageBox::Critical, "Failed to export module",
                               "The file you selected couldn't be opened.", QMessageBox::Ok);
            AxiomUtil::showMessageBox(msgBox);
            return;
        }

        QDataStream stream(&file);
        AxiomModel::ProjectSerializer::writeHeader(stream, AxiomModel::ProjectSerializer::librarySchemaMagic);
        AxiomModel::LibrarySerializer::serializeEntries(1, &_entry, &_entry + 1, stream);
        file.close();
    } else if (selectedAction == deleteAction) {
        QMessageBox confirmBox(QMessageBox::Warning, "Confirm Delete",
                               "Are you sure you want to delete this module?\n\n"
                               "This operation cannot be undone.");
        confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        confirmBox.setDefaultButton(QMessageBox::Yes);

        if (AxiomUtil::showMessageBox(confirmBox) == QMessageBox::Yes) {
            _entry->remove();
        }
    }
}

void ModulePreviewButton::setName(QString name) {
    QFontMetrics metrics(label->font());
    auto elidedText = metrics.elidedText(name, Qt::ElideRight, label->width());
    label->setText(elidedText);
}

void ModulePreviewButton::updateIsVisible() {
    auto hasTag = library->activeTag() == "" || _entry->tags().find(library->activeTag()) != _entry->tags().end();
    auto hasSearch =
        library->activeSearch() == "" || _entry->name().contains(library->activeSearch(), Qt::CaseInsensitive);
    setVisible(hasTag && hasSearch);
}

void ModulePreviewButton::updateImage() {
    ModulePreviewCanvas canvas(_entry->rootSurface(), window->runtime());
    QGraphicsView view(&canvas);
    view.setInteractive(false);
    view.setRenderHint(QPainter::Antialiasing);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setFixedSize(100, 100);
    view.setStyleSheet("* { border: none; background-color: #111; }");

    // figure out the bounding box size of the scene
    QRectF boundingRect;
    for (const auto &item : view.scene()->items()) {
        if (auto node = dynamic_cast<NodeItem *>(item)) {
            auto br = node->drawBoundingRect();
            br.moveTopLeft(node->scenePos());
            boundingRect = boundingRect.united(br);
        }
    }
    view.setSceneRect(boundingRect);

    // figure out correct scaling with some padding
    auto viewWidth = view.width() - 30;
    auto viewHeight = view.height() - 30;
    auto scaleFactor = boundingRect.width() > boundingRect.height() ? viewWidth / boundingRect.width()
                                                                    : viewHeight / boundingRect.height();
    view.centerOn(boundingRect.center());
    view.scale(scaleFactor, scaleFactor);

    image->setPixmap(view.grab());
}
