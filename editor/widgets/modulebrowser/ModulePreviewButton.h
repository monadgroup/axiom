#pragma once

#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

#include "common/Hookable.h"

namespace AxiomModel {
    class Library;

    class LibraryEntry;
}

namespace AxiomGui {

    class MainWindow;

    class ModulePreviewButton : public QFrame, public AxiomCommon::Hookable {
    Q_OBJECT

    public:
        explicit ModulePreviewButton(MainWindow *window, AxiomModel::Library *library, AxiomModel::LibraryEntry *entry,
                                     QWidget *parent = nullptr);

    private:
        AxiomModel::Library *library;
        AxiomModel::LibraryEntry *entry;
        QLabel *label;

        void setName(QString name);

        void updateIsVisible();

    };

}
