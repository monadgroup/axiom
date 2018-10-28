#pragma once

#include "Node.h"

namespace AxiomModel {

    class PortalNode : public Node {
    public:
        PortalNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                   const QUuid &controlsUuid, ModelRoot *root);

        static std::unique_ptr<PortalNode> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                  bool selected, QString name, const QUuid &controlsUuid,
                                                  ModelRoot *root);

        QString debugName() override;

        bool isResizable() const override { return false; }

        bool isCopyable() const override { return false; }

        bool isDeletable() const override { return true; }

        void attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) override {}
    };
}
