#pragma once

#include <QtWidgets/QScrollArea>

#include "common/Hookable.h"

namespace AxiomModel {
    class Library;

    class LibraryEntry;
};

namespace AxiomGui {

    class FlowLayout;

    class MainWindow;

    class ModulePreviewList : public QScrollArea, public AxiomCommon::Hookable {
    Q_OBJECT

    public:
        explicit ModulePreviewList(MainWindow *window, AxiomModel::Library *library, QWidget *parent = nullptr);

    private slots:

        void addEntry(AxiomModel::LibraryEntry *entry);

    private:

        MainWindow *window;
        AxiomModel::Library *library;
        FlowLayout *layout;
    };

}
