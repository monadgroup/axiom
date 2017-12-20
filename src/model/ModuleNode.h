#pragma once
#include <memory>
#include <QtCore/QObject>

#include "Node.h"

namespace AxiomModel {

    class ModuleSchematic;

    class ModuleNode : public Node {
        Q_OBJECT

    public:
        std::unique_ptr<ModuleSchematic> schematic;
    };

}
