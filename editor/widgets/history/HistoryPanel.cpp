#include "HistoryPanel.h"

#include "editor/model/HistoryList.h"

using namespace AxiomGui;

HistoryPanel::HistoryPanel(AxiomModel::HistoryList *list, QWidget *parent) : DockPanel("History", parent), list(list) {
    listWidget = new QListWidget(this);
    setWidget(listWidget);

    //connect(list, &AxiomModel::HistoryList::stackChanged,
    //        this, &HistoryPanel::updateStack);
    updateStack();
}

void HistoryPanel::updateStack() {
    /*listWidget->clear();
    for (size_t i = 0; i < list->stack().size(); i++) {
        auto item = new QListWidgetItem(AxiomModel::HistoryList::typeToString(list->stack()[i].type), listWidget);
        auto itemBrush = i >= list->stackPos() ? QBrush(Qt::gray) : QBrush(Qt::white);
        item->setForeground(itemBrush);
        listWidget->addItem(item);

        for (const auto &op : list->stack()[i].operations) {
            auto opItem = new QListWidgetItem("  " + AxiomModel::HistoryOperation::typeToString(op->type()), listWidget);
            opItem->setForeground(itemBrush);
            listWidget->addItem(opItem);
        }
    }*/
}
