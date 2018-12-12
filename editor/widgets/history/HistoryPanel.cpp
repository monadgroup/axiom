#include "HistoryPanel.h"

#include <QtWidgets/QListWidget>

using namespace AxiomGui;

HistoryPanel::HistoryPanel(QWidget *parent) : ads::CDockWidget("History", parent) {
    listWidget = new QListWidget(this);
    setWidget(listWidget);
}

void HistoryPanel::setSource(AxiomModel::HistoryList *list) {
    currentContext.emplace(listWidget, list);
}
