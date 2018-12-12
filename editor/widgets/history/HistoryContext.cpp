#include "HistoryContext.h"

#include <QtWidgets/QListWidget>

#include "editor/model/HistoryList.h"
#include "editor/model/actions/CompositeAction.h"

using namespace AxiomGui;

HistoryContext::HistoryContext(QListWidget *listWidget, AxiomModel::HistoryList *historySource)
    : listWidget(listWidget), historySource(historySource) {
    historySource->stackChanged.connectTo(this, &HistoryContext::updateStack);
    updateStack();
}

void HistoryContext::updateStack() {
    listWidget->clear();
    for (size_t i = 0; i < historySource->stack().size(); i++) {
        appendItem(i, historySource->stack()[i].get(), "");
    }
}

void HistoryContext::appendItem(size_t index, AxiomModel::Action *action, const QString &prepend) {
    auto item = new QListWidgetItem(prepend + AxiomModel::Action::typeToString(action->actionType()), listWidget);
    auto itemBrush = index >= historySource->stackPos() ? QBrush(Qt::gray) : QBrush(Qt::white);
    item->setForeground(itemBrush);
    listWidget->addItem(item);

    if (auto composite = dynamic_cast<AxiomModel::CompositeAction *>(action)) {
        for (const auto &subAction : composite->actions()) {
            appendItem(index, subAction.get(), prepend + "  ");
        }
    }
}
