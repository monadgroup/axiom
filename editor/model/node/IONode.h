#pragma once

#include <QtCore/QObject>

#include "compiler/runtime/IONode.h"
#include "Node.h"

namespace MaximRuntime {
    class IONode;
}

namespace AxiomModel {

    class RootSchematic;

    class NodeIOControl;

    class IONode : public Node {
    Q_OBJECT

    public:
        IONode(RootSchematic *parent, const QString &name, QPoint pos, MaximCommon::ControlType type);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

        bool isResizable() const override { return false; }

        bool isCopyable() const override { return false; }

        bool isDeletable() const override { return false; }

        MaximRuntime::IONode *runtime() override { return _runtime; }

        void attachRuntime(MaximRuntime::IONode *runtime);

        void createAndAttachRuntime(MaximRuntime::Surface *surface) { }

    private:

        NodeIOControl *_control;
        MaximRuntime::IONode *_runtime = nullptr;
    };

}
