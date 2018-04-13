#include "ModulePreviewButton.h"

#include <QtWidgets/QGridLayout>

#include "ModulePreviewView.h"
#include "editor/model/LibraryEntry.h"

using namespace AxiomGui;

ModulePreviewButton::ModulePreviewButton(AxiomModel::LibraryEntry *entry, QWidget *parent) : QFrame(parent) {
    auto mainLayout = new QGridLayout(this);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setRowStretch(0, 1);

    mainLayout->addWidget(new ModulePreviewView(entry, this), 0, 0);

    label = new QLabel(this);
    mainLayout->addWidget(label, 1, 0);
    label->setMargin(0);
    label->setContentsMargins(5, 0, 0, 0);

    setLayout(mainLayout);

    connect(entry, &AxiomModel::LibraryEntry::nameChanged,
            this, &ModulePreviewButton::setName);
    setName(entry->name());
}

void ModulePreviewButton::setName(QString name) {
    label->setText(name);
}
