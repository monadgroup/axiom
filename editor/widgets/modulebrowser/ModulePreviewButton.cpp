#include "ModulePreviewButton.h"

#include <QtWidgets/QGridLayout>
#include <QtGui/QContextMenuEvent>

#include "editor/model/Library.h"
#include "editor/model/LibraryEntry.h"
#include "ModulePreviewView.h"

using namespace AxiomGui;

ModulePreviewButton::ModulePreviewButton(MainWindow *window, AxiomModel::Library *library,
                                         AxiomModel::LibraryEntry *entry, QWidget *parent) : QFrame(parent),
                                                                                             library(library),
                                                                                             entry(entry) {
    setFixedWidth(110);

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

    library->activeTagChanged.connect(this, &ModulePreviewButton::updateIsVisible);
    library->activeSearchChanged.connect(this, &ModulePreviewButton::updateIsVisible);
    updateIsVisible();
}

void ModulePreviewButton::setName(QString name) {
    QFontMetrics metrics(label->font());
    auto elidedText = metrics.elidedText(name, Qt::ElideRight, label->width());
    label->setText(elidedText);
}

void ModulePreviewButton::updateIsVisible() {
    auto hasTag = library->activeTag() == "" || entry->tags().find(library->activeTag()) != entry->tags().end();
    auto hasSearch = library->activeSearch() == "" || entry->name().contains(library->activeSearch(), Qt::CaseInsensitive);
    setVisible(hasTag && hasSearch);
}
