#include "AddNodeMenu.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>

#include "editor/model/schematic/Schematic.h"
#include "editor/model/Project.h"
#include "editor/model/LibraryEntry.h"

using namespace AxiomGui;

AddNodeMenu::AddNodeMenu(AxiomModel::Schematic *schematic, const QString &search) : schematic(schematic) {
    contextSearch = new QLineEdit(this);
    contextSearch->setPlaceholderText("Search modules...");
    contextSearch->setText(search);
    connect(contextSearch, &QLineEdit::textChanged,
            this, &AddNodeMenu::applySearch);

    auto widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(contextSearch);
    addAction(widgetAction);
    addSeparator();

    auto newNodeAction = addAction(tr("New Node"));
    connect(newNodeAction, &QAction::triggered,
            this, &AddNodeMenu::newNodeAdded);

    auto newGroupAction = addAction(tr("New Group"));
    connect(newGroupAction, &QAction::triggered,
            this, &AddNodeMenu::newGroupAdded);

    addSeparator();

    // add default entries
    std::vector<AxiomModel::LibraryEntry *> sortedEntries;
    for (const auto &entry : schematic->project()->library.entries()) {
        sortedEntries.push_back(entry.get());
    }
    std::sort(sortedEntries.begin(), sortedEntries.end(), [](AxiomModel::LibraryEntry *a, AxiomModel::LibraryEntry *b) {
        return a->name() < b->name();
    });
    for (const auto &entry : sortedEntries) {
        entryActions.emplace(entry, addAction(entry->name()));
    }

    cantFindAction = addAction(tr("Oops, I can't find that one..."));
    cantFindAction->setEnabled(false);
    cantFindAction->setVisible(false);

    applySearch(search);
}

void AddNodeMenu::applySearch(QString search) {
    QString lowerSearch = search.toLower();

    bool anyEntryVisible = false;
    for (const auto &pair : entryActions) {
        auto inName = pair.first->name().toLower().contains(lowerSearch);
        pair.second->setVisible(inName);
        if (inName) anyEntryVisible = true;
    }

    cantFindAction->setVisible(!anyEntryVisible);
}
