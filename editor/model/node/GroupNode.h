#pragma once

#include <memory>
#include <QtCore/QObject>

#include "editor/model/schematic/GroupSchematic.h"
#include "Node.h"
#include "compiler/runtime/GroupNode.h"

namespace AxiomModel {

    class GroupNode : public Node {
    Q_OBJECT

    public:
        std::unique_ptr<GroupSchematic> schematic;

        GroupNode(Schematic *parent, QString name, QPoint pos, QSize size);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

        MaximRuntime::GroupNode *runtime() override { return _runtime; }

        void attachRuntime(MaximRuntime::GroupNode *runtime);

        void createAndAttachRuntime(MaximRuntime::Surface *surface) override;

    public slots:

        void saveValue() override;

        void restoreValue() override;

        void serialize(QDataStream &stream, QPoint offset) const override;

        void deserialize(QDataStream &stream, QPoint offset) override;

    private slots:

        void controlAdded(MaximRuntime::Control *control);

    private:

        MaximRuntime::GroupNode *_runtime = nullptr;
    };

}
