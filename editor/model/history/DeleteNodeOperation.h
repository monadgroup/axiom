#pragma once

#include "HistoryOperation.h"
#include "../Project.h"
#include "../node/Node.h"

namespace AxiomModel {

    class DeleteNodeOperation : public HistoryOperation {
    public:
        DeleteNodeOperation(Project *project, NodeRef nodeRef);

        void forward() override;

        void backward() override;

    private:

        Project *project;
        NodeRef nodeRef;
        Node::Type nodeType;
        std::unique_ptr<QByteArray> nodeBuffer;
    };

}
