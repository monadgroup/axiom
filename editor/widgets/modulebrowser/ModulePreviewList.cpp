#include "ModulePreviewList.h"

#include "../layouts/FlowLayout.h"
#include "editor/util.h"
#include "ModulePreviewButton.h"
#include "editor/model/Library.h"
#include "editor/model/LibraryEntry.h"

using namespace AxiomGui;

ModulePreviewList::ModulePreviewList(MainWindow *window, AxiomModel::Library *library, QWidget *parent) : QScrollArea(parent), window(window), library(library) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/ModulePreviewList.qss"));

    auto widget = new QWidget(this);
    layout = new FlowLayout(this, 0, 0, 0);

    for (const auto &entry : library->entries()) {
        addEntry(entry.get());
    }

    connect(library, &AxiomModel::Library::entryAdded,
            this, &ModulePreviewList::addEntry);

    widget->setLayout(layout);
    setWidgetResizable(true);
    setWidget(widget);
}

void ModulePreviewList::addEntry(AxiomModel::LibraryEntry *entry) {
    auto widget = new ModulePreviewButton(window, library, entry, this);
    widget->setObjectName("preview-button");
    layout->addWidget(widget);

    connect(entry, &AxiomModel::LibraryEntry::removed,
            [this, widget]() {
                layout->removeWidget(widget);
                delete widget;
            });
}
