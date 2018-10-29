#pragma once

#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

#include "common/TrackedObject.h"

namespace AxiomModel {
    class Library;

    class LibraryEntry;
}

namespace AxiomGui {

    class MainWindow;

    class ModulePreviewButton : public QFrame, public AxiomCommon::TrackedObject {
        Q_OBJECT

    public:
        explicit ModulePreviewButton(MainWindow *window, AxiomModel::Library *library, AxiomModel::LibraryEntry *entry,
                                     QWidget *parent = nullptr);

        AxiomModel::LibraryEntry *entry() { return _entry; }

    private:
        AxiomModel::Library *library;
        AxiomModel::LibraryEntry *_entry;
        QLabel *label;

        void setName(QString name);

        void updateIsVisible();
    };
}
