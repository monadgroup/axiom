#pragma once

#include <QtCore/QString>
#include <QtCore/QObject>

#include "history/HistoryList.h"
#include "Library.h"
#include "schematic/RootSchematic.h"
#include "Ref.h"

namespace MaximRuntime {

    class Runtime;

}

namespace AxiomModel {

    class Node;
    class NodeControl;

    class Project {
    public:
        HistoryList history;
        Library library;
        RootSchematic root;

        explicit Project(MaximRuntime::Runtime *runtime);

        void serialize(QDataStream &stream) const;

        void deserialize(QDataStream &stream);

        void load(QDataStream &stream);

        void clear();

        void build();

        Schematic *findSurface(const SurfaceRef &ref);

        Node *findNode(const NodeRef &ref);

        NodeControl *findControl(const ControlRef &ref);

        MaximRuntime::Runtime *runtime() const { return _runtime; }

    private:

        MaximRuntime::Runtime *_runtime;

    };

}
