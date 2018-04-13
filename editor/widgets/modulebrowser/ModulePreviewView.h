#pragma once

#include <QtWidgets/QGraphicsView>

namespace AxiomModel {
    class LibraryEntry;
}

namespace AxiomGui {

    class ModulePreviewView : public QGraphicsView {
    Q_OBJECT

    public:
        explicit ModulePreviewView(AxiomModel::LibraryEntry *entry, QWidget *parent = nullptr);

    private:

        AxiomModel::LibraryEntry *entry;
    };

}
