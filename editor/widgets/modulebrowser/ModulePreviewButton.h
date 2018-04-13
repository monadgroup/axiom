#pragma once

#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

namespace AxiomModel {
    class LibraryEntry;
}

namespace AxiomGui {

    class ModulePreviewButton : public QFrame {
    Q_OBJECT

    public:
        explicit ModulePreviewButton(AxiomModel::LibraryEntry *entry, QWidget *parent = nullptr);

    private slots:

        void setName(QString name);

    private:
        QLabel *label;

    };

}
