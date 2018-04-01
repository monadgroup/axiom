#pragma once

#include <QtCore/QString>
#include <QtCore/QObject>

#include "Library.h"
#include "editor/model/schematic/RootSchematic.h"

namespace MaximRuntime {

    class Runtime;

}

namespace AxiomModel {

    class Project {
    public:
        Library library;
        RootSchematic root;

        explicit Project(MaximRuntime::Runtime *runtime);

        void serialize(QDataStream &stream) const;

        void deserialize(QDataStream &stream);

        void clear();

        MaximRuntime::Runtime *runtime() const { return _runtime; }

    private:

        MaximRuntime::Runtime *_runtime;

    };

}
