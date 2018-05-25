#include "ModulePreviewButton.h"

#include <QtWidgets/QGridLayout>
#include <QtGui/QContextMenuEvent>

#include "editor/model/Library.h"
#include "editor/model/LibraryEntry.h"
#include "ModulePreviewView.h"

using namespace AxiomGui;

ModulePreviewButton::ModulePreviewButton(MainWindow *window, AxiomModel::Library *library,
                                         AxiomModel::LibraryEntry *entry, QWidget *parent) : QFrame(parent),
                                                                                             entry(entry) {
    auto mainLayout = new QGridLayout(this);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setRowStretch(0, 1);

    mainLayout->addWidget(new ModulePreviewView(window, library, entry, this), 0, 0);

    label = new QLabel(this);
    mainLayout->addWidget(label, 1, 0);
    label->setMargin(0);
    label->setContentsMargins(5, 0, 0, 0);

    setLayout(mainLayout);

    entry->nameChanged.connect(this, &ModulePreviewButton::setName);
    setName(entry->name());

    library->activeTagChanged.connect(this, &ModulePreviewButton::setVisibleTag);
    setVisibleTag(library->activeTag());
}

void ModulePreviewButton::setName(QString name) {
    label->setText(name);
}

void ModulePreviewButton::setVisibleTag(const QString &tag) {
    auto hasTag = tag == "" || entry->tags().find(tag) != entry->tags().end();
    setVisible(hasTag);
}
