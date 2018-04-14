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

    protected:
        void mousePressEvent(QMouseEvent *event) override;

    private:

        AxiomModel::LibraryEntry *entry;
    };

}
