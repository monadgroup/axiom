#include "ModulePreviewList.h"

#include "../layouts/FlowLayout.h"
#include "ModulePreviewButton.h"
#include "editor/model/Library.h"
#include "editor/model/LibraryEntry.h"
#include "editor/util.h"

using namespace AxiomGui;

ModulePreviewList::ModulePreviewList(MainWindow *window, AxiomModel::Library *library, QWidget *parent)
    : QScrollArea(parent), window(window), library(library) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/styles/ModulePreviewList.qss"));

    auto widget = new QWidget(this);
    layout = new FlowLayout(this, 0, 0, 0);

    for (const auto &entry : library->entries()) {
        addEntry(entry);
    }

    library->entryAdded.connect(this, &ModulePreviewList::addEntry);

    widget->setLayout(layout);
    setWidgetResizable(true);
    setWidget(widget);
}

void ModulePreviewList::addEntry(AxiomModel::LibraryEntry *entry) {
    auto widget = new ModulePreviewButton(window, library, entry, this);
    widget->setObjectName("preview-button");
    layout->addWidget(widget);
    entry->removed.connect(this, [this, widget]() {
        layout->removeWidget(widget);
        delete widget;
    });
}
