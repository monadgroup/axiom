#pragma once

#include <QtCore/QObject>

#include "compiler/runtime/OutputNode.h"
#include "Node.h"

namespace MaximRuntime {
    class OutputNode;
}

namespace AxiomModel {

    class OutputNode : public Node {
        Q_OBJECT

    public:
        OutputNode(Schematic *parent, QPoint pos);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

        bool isResizable() const override { return false; }

        bool isDeletable() const override { return false; }

        MaximRuntime::OutputNode *runtime() override { return &_runtime; }

    private:

        MaximRuntime::OutputNode _runtime;
    };

}
