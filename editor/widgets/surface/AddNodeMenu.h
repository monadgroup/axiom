#pragma once

#include <QtWidgets/QMenu>

class QLineEdit;

namespace AxiomModel {
    class Schematic;

    class LibraryEntry;
}

namespace AxiomGui {

    class AddNodeMenu : public QMenu {
    Q_OBJECT

    public:
        AddNodeMenu(AxiomModel::Schematic *schematic, const QString &search);

    public slots:

        void applySearch(QString search);

    signals:

        void newNodeAdded();

        void newGroupAdded();

        void moduleAdded(AxiomModel::LibraryEntry *entry);

    private:

        AxiomModel::Schematic *schematic;
        QLineEdit *contextSearch;

        std::map<AxiomModel::LibraryEntry *, QAction *> entryActions;
        QAction *cantFindAction;
    };

}
