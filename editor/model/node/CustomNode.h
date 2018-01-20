#pragma once

#include <QtCore/QObject>

#include "Node.h"

namespace AxiomModel {

    class CustomNode : public Node {
    Q_OBJECT

    public:
        CustomNode(Schematic *parent, QString name, QPoint pos, QSize);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;
    };

}
