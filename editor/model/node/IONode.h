#pragma once

#include <QtCore/QObject>

#include "compiler/runtime/IONode.h"
#include "Node.h"

namespace MaximRuntime {
    class IONode;
}

namespace AxiomModel {

    class RootSchematic;

    class IONode : public Node {
    Q_OBJECT

    public:
        IONode(RootSchematic *parent, MaximRuntime::IONode *runtime, const QString &name, QPoint pos);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

        bool isResizable() const override { return false; }

        bool isDeletable() const override { return false; }

        MaximRuntime::IONode *runtime() override { return _runtime; }

    private:

        MaximRuntime::IONode *_runtime;
    };

}
