#pragma once

#include "Node.h"

namespace MaximRuntime {
    class IONode;
}

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

        const std::optional<MaximRuntime::IONode *> &runtime() const { return _runtime; }

        void createAndAttachRuntime(MaximRuntime::Surface *parent) override;

        void attachRuntime(MaximRuntime::IONode *runtime);

        void detachRuntime();

        bool isResizable() const override { return false; }

        bool isCopyable() const override { return false; }

        bool isDeletable() const override { return true; }

    private:
        std::optional<MaximRuntime::IONode *> _runtime;
    };

}
