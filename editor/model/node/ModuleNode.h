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

        ModuleNode(Schematic *parent, QString name, QPoint pos, QSize size);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;
    };

}