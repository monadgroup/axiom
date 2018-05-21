#include "HistoryPanel.h"

#include "editor/model/HistoryList.h"
#include "editor/model/actions/CompositeAction.h"

using namespace AxiomGui;

HistoryPanel::HistoryPanel(AxiomModel::HistoryList *list, QWidget *parent) : DockPanel("History", parent), list(list) {
    listWidget = new QListWidget(this);
    setWidget(listWidget);

    list->stackChanged.connect(this, &HistoryPanel::updateStack);
    updateStack();
}

void HistoryPanel::updateStack() {
    listWidget->clear();
    for (size_t i = 0; i < list->stack().size(); i++) {
        appendItem(i, list->stack()[i].get(), "");
    }
}

void HistoryPanel::appendItem(size_t i, AxiomModel::Action *action, QString prepend) {
    auto item = new QListWidgetItem(prepend + AxiomModel::Action::typeToString(action->actionType()), listWidget);
    auto itemBrush = i >= list->stackPos() ? QBrush(Qt::gray) : QBrush(Qt::white);
    item->setForeground(itemBrush);
    listWidget->addItem(item);

    if (auto composite = dynamic_cast<AxiomModel::CompositeAction*>(action)) {
        for (const auto &subAction : composite->actions()) {
            appendItem(i, subAction.get(), prepend + "  ");
        }
    }
}
