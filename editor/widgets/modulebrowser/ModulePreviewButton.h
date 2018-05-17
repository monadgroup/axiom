#pragma once

#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

namespace AxiomModel {
    class Library;

    class LibraryEntry;
}

namespace AxiomGui {

    class MainWindow;

    class ModulePreviewButton : public QFrame {
    Q_OBJECT

    public:
        explicit ModulePreviewButton(MainWindow *window, AxiomModel::Library *library, AxiomModel::LibraryEntry *entry,
                                     QWidget *parent = nullptr);

    private slots:

        void setName(QString name);

        void setVisibleTag(const QString &tag);

    private:
        AxiomModel::LibraryEntry *entry;
        QLabel *label;

    };

}
