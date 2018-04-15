#pragma once

#include <QtWidgets/QListWidget>

#include "../dock/DockPanel.h"

namespace AxiomModel {
    class HistoryList;
}

namespace AxiomGui {

    class HistoryPanel : public DockPanel {
    Q_OBJECT

    public:
        explicit HistoryPanel(AxiomModel::HistoryList *list, QWidget *parent = nullptr);

    private slots:

        void updateStack();

    private:

        AxiomModel::HistoryList *list;
        QListWidget *listWidget;
    };

}
