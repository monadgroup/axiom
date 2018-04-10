#pragma once

#include "HistoryOperation.h"
#include "../node/Node.h"

namespace AxiomModel {

    class DeleteNodeOperation : public HistoryOperation {
    public:
        DeleteNodeOperation(Project *project, NodeRef nodeRef);

        static std::unique_ptr<DeleteNodeOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        NodeRef nodeRef;
        Node::Type nodeType;
        std::unique_ptr<QByteArray> nodeBuffer;
    };

}
