#pragma once

#include <QtWidgets/QGraphicsView>

namespace AxiomModel {
    class Library;

    class LibraryEntry;
}

namespace AxiomGui {

    class MainWindow;

    class ModulePreviewView : public QGraphicsView {
    Q_OBJECT

    public:
        explicit ModulePreviewView(MainWindow *window, AxiomModel::Library *library, AxiomModel::LibraryEntry *entry,
                                   QWidget *parent = nullptr);

    protected:
        void mousePressEvent(QMouseEvent *event) override;

        void mouseDoubleClickEvent(QMouseEvent *event) override;

        void contextMenuEvent(QContextMenuEvent *event) override;

    private slots:

        void updateScaling();

    private:

        MainWindow *window;
        AxiomModel::Library *library;
        AxiomModel::LibraryEntry *entry;
        qreal currentScale = 1;
    };

}
