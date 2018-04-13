#include "ModulePreviewList.h"

#include "../layouts/FlowLayout.h"
#include "editor/util.h"
#include "ModulePreviewButton.h"
#include "editor/model/Library.h"
#include "editor/model/LibraryEntry.h"

using namespace AxiomGui;

ModulePreviewList::ModulePreviewList(AxiomModel::Library *library, QWidget *parent) : QScrollArea(parent), library(library) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/ModulePreviewList.qss"));

    auto widget = new QWidget(this);
    layout = new FlowLayout(this, -1, 10, 10);

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
    layout->addWidget(new ModulePreviewButton(library, entry, this));
}
