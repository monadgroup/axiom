#pragma once

#include <QtCore/QObject>

#include "Node.h"

namespace AxiomModel {

    class OutputNode : public Node {
        Q_OBJECT

    public:
        OutputNode(Schematic *parent, QPoint pos, QSize size);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;


    };

}
