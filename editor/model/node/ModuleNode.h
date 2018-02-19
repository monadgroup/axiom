#pragma once

#include <memory>
#include <QtCore/QObject>

#include "editor/model/schematic/ModuleSchematic.h"
#include "Node.h"
#include "compiler/runtime/GroupNode.h"

namespace AxiomModel {

    class ModuleNode : public Node {
    Q_OBJECT

    public:
        std::unique_ptr<ModuleSchematic> schematic;

        ModuleNode(Schematic *parent, QString name, QPoint pos, QSize size);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

        MaximRuntime::GroupNode *runtime() { return &_node; }

    private:

        MaximRuntime::GroupNode _node;
    };

}
