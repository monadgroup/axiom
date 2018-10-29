#pragma once

#include <QtWidgets/QScrollArea>

#include "../layouts/FlowLayout.h"
#include "common/TrackedObject.h"

namespace AxiomModel {
    class Library;

    class LibraryEntry;
};

namespace AxiomGui {

    class MainWindow;

    class ModulePreviewList : public QScrollArea, public AxiomCommon::TrackedObject {
        Q_OBJECT

    public:
        explicit ModulePreviewList(MainWindow *window, AxiomModel::Library *library, QWidget *parent = nullptr);

    private slots:

        void addEntry(AxiomModel::LibraryEntry *entry);

    private:
        class Sorter : public FlowLayoutSorter {
        public:
            bool aAfterB(QLayoutItem *a, QLayoutItem *b) override;
        };

        Sorter sorter;
        MainWindow *window;
        AxiomModel::Library *library;
        FlowLayout *layout;
    };
}
