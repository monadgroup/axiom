#pragma once

#include <memory>
#include <QtCore/QObject>

#include "editor/model/schematic/ModuleSchematic.h"
#include "Node.h"

namespace AxiomModel {

    class ModuleNode : public Node {
    Q_OBJECT

    public:
        std::unique_ptr<ModuleSchematic> schematic;

        explicit ModuleNode(Schematic *parent);
    };

}
