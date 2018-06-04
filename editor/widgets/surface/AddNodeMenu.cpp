#include "AddNodeMenu.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>

#include "editor/model/Project.h"
#include "editor/model/LibraryEntry.h"
#include "editor/model/objects/NodeSurface.h"

using namespace AxiomGui;

AddNodeMenu::AddNodeMenu(AxiomModel::NodeSurface *surface, const QString &search) : surface(surface) {
    setStyleSheet("QMenu { menu-scrollable: 1; }");

    /*contextSearch = new QLineEdit(this);
    contextSearch->setPlaceholderText("Search modules...");
    contextSearch->setText(search);
    connect(contextSearch, &QLineEdit::textChanged,
            this, &AddNodeMenu::applySearch);

    auto widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(contextSearch);
    addAction(widgetAction);
    addSeparator();*/

    auto newNodeAction = addAction(tr("New Node"));
    connect(newNodeAction, &QAction::triggered,
            this, &AddNodeMenu::newNodeAdded);

    auto newGroupAction = addAction(tr("New Group"));
    connect(newGroupAction, &QAction::triggered,
            this, &AddNodeMenu::newGroupAdded);

    if (surface->canHaveAutomation()) {
        auto newAutomationAction = addAction(tr("New Automation"));
        connect(newAutomationAction, &QAction::triggered,
                this, &AddNodeMenu::newAutomationAdded);
    }

    /*addSeparator();

    // add default entries
    std::vector<AxiomModel::LibraryEntry *> sortedEntries = surface->root()->project()->library().entries();
    std::sort(sortedEntries.begin(), sortedEntries.end(), [](AxiomModel::LibraryEntry *a, AxiomModel::LibraryEntry *b) {
        return a->name() < b->name();
    });
    for (size_t i = 0; i < sortedEntries.size(); i++) {
        auto &entry = sortedEntries[i];
        auto action = addAction(entry->name());
        action->setVisible(i < 20);
        entryActions.emplace(entry, action);
    }

    cantFindAction = addAction(tr("Oops, I can't find that one..."));
    cantFindAction->setEnabled(false);
    cantFindAction->setVisible(false);

    applySearch(search);*/
}

void AddNodeMenu::applySearch(QString search) {
    QString lowerSearch = search.toLower();

    size_t visibleCount = 0;
    for (const auto &pair : entryActions) {
        auto inName = pair.first->name().toLower().contains(lowerSearch);
        pair.second->setVisible(visibleCount < 20 && inName);
        if (inName) visibleCount++;
    }

    cantFindAction->setVisible(!visibleCount && !entryActions.empty());
}
