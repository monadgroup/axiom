#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

#include "Schematic.h"

namespace MaximRuntime {
    class RootSchematic;
}

namespace AxiomModel {

    class RootSchematic : public Schematic {
    Q_OBJECT

    public:
        explicit RootSchematic(MaximRuntime::RootSchematic *runtime);

        QString name() override;
    };

}
