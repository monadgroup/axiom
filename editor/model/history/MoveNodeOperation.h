#pragma once

#include "HistoryOperation.h"
#include "../node/Node.h"

namespace AxiomModel {

    class MoveNodeOperation : public HistoryOperation {
    public:
        MoveNodeOperation(Project *project, NodeRef nodeRef, QPoint startPos, QPoint endPos);

        static std::unique_ptr<MoveNodeOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        NodeRef nodeRef;
        QPoint startPos;
        QPoint endPos;
    };

}
