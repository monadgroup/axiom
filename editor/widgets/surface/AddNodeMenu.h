#pragma once

#include "editor/model/ConnectionWire.h"
#include "editor/model/objects/PortalControl.h"
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
        AddNodeMenu(AxiomModel::NodeSurface *surface, const QString &search, QWidget *parent = nullptr);

    public slots:

        void applySearch(QString search);

    signals:

        void newNodeAdded();

        void newGroupAdded();

        void newPortalAdded(AxiomModel::PortalControl::PortalType portalType,
                            AxiomModel::ConnectionWire::WireType wireType);

        void moduleAdded(AxiomModel::LibraryEntry *entry);

    private:
        AxiomModel::NodeSurface *surface;
        QLineEdit *contextSearch;

        std::map<AxiomModel::LibraryEntry *, QAction *> entryActions;
        QAction *cantFindAction;
    };
}
