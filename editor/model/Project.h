#pragma once

#include <QtCore/QString>
#include <QtCore/QObject>

#include "Library.h"
#include "editor/model/schematic/RootSchematic.h"

namespace AxiomModel {

    class Project {
    public:
        QString path;

        Library library;
        RootSchematic root;

        void serialize(QDataStream &stream) const;

        void deserialize(QDataStream &stream);
    };

}
