#pragma once

#include <QtWidgets/QMenu>

class QLineEdit;

namespace AxiomModel {
    class NodeSurface;

    class LibraryEntry;
}

namespace AxiomGui {

    class AddNodeMenu : public QMenu {
    Q_OBJECT

    public:
        AddNodeMenu(AxiomModel::NodeSurface *surface, const QString &search);

    public slots:

        void applySearch(QString search);

    signals:

        void newNodeAdded();

        void newGroupAdded();

        void newAutomationAdded();

        void moduleAdded(AxiomModel::LibraryEntry *entry);

    private:

        AxiomModel::NodeSurface *surface;
        QLineEdit *contextSearch;

        std::map<AxiomModel::LibraryEntry *, QAction *> entryActions;
        QAction *cantFindAction;
    };

}
