#pragma once

#include "Node.h"

namespace AxiomModel {

    class AutomationNode : public Node {
    public:
        AutomationNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       const QUuid &controlsUuid, ModelRoot *root);

        static std::unique_ptr<AutomationNode>
        create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
               const QUuid &controlsUuid, ModelRoot *root);

        static std::unique_ptr<AutomationNode>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                    bool selected, QString name, const QUuid &controlsUuid, ReferenceMapper *ref, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        bool isResizable() const override { return false; }

        bool isCopyable() const override { return false; }

        bool isDeletable() const override { return true; }

        void attachRuntime(MaximCompiler::Runtime *runtime) override {}
    };

}
