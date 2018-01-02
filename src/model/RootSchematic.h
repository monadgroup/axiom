#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

#include "Schematic.h"

namespace AxiomModel {

    class RootSchematic : public Schematic {
    Q_OBJECT

    public:
        QString name() override;
    };

}
