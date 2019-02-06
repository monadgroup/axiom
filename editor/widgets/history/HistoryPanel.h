#pragma once

#include <optional>

#include "HistoryContext.h"
#include "vendor/dock/DockWidget.h"

class QListWidget;

namespace AxiomModel {
    class HistoryList;
}

namespace AxiomGui {

    class HistoryPanel : public ads::CDockWidget {
        Q_OBJECT

    public:
        explicit HistoryPanel(QWidget *parent = nullptr);

        void setSource(AxiomModel::HistoryList *list);

    private:
        QListWidget *listWidget;
        std::optional<HistoryContext> currentContext;
    };
}
