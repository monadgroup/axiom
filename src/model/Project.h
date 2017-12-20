#pragma once
#include <QtCore/QString>
#include <QtCore/QObject>

#include "Library.h"
#include "RootSchematic.h"

namespace AxiomModel {

    class Project {
        Q_OBJECT

    public:
        QString path;

        Library library;
        RootSchematic root;
    };

}
