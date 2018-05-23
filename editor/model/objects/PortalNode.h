#pragma once

#include "Node.h"
#include "PortalControl.h"
#include "../ConnectionWire.h"

namespace MaximRuntime {
    class IONode;
}

namespace AxiomModel {

    class PortalNode : public Node {
    public:
        PortalNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                   const QUuid &controlsUuid, ModelRoot *root);

        static std::unique_ptr<PortalNode>
        create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
               const QUuid &controlsUuid, ModelRoot *root);

        static std::unique_ptr<PortalNode>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                    bool selected, QString name, const QUuid &controlsUuid, ReferenceMapper *ref, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        void createAndAttachRuntime(MaximRuntime::Surface *parent) override {}

        void attachRuntime(MaximRuntime::IONode *runtime);

        bool isResizable() const override { return false; }

        bool isCopyable() const override { return false; }

        bool isDeletable() const override { return false; }
    };

}
