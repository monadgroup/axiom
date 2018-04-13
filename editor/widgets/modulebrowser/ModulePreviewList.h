#pragma once

#include <QtWidgets/QScrollArea>

namespace AxiomModel {
    class Library;

    class LibraryEntry;
};

namespace AxiomGui {

    class FlowLayout;

    class ModulePreviewList : public QScrollArea {
    Q_OBJECT

    public:
        explicit ModulePreviewList(AxiomModel::Library *library, QWidget *parent = nullptr);

    private slots:

        void addEntry(AxiomModel::LibraryEntry *entry);

    private:

        FlowLayout *layout;
    };

}
