#pragma once

#include "common/TrackedObject.h"

class QString;

class QListWidget;

namespace AxiomModel {
    class Action;

    class HistoryList;
}

namespace AxiomGui {

    class HistoryContext : public AxiomCommon::TrackedObject {
    public:
        HistoryContext(QListWidget *listWidget, AxiomModel::HistoryList *historySource);

    private:
        QListWidget *listWidget;
        AxiomModel::HistoryList *historySource;

        void updateStack();
        void appendItem(size_t index, AxiomModel::Action *action, const QString &prepend);
    };
}
