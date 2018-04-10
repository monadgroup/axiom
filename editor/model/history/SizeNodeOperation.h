#pragma once

#include "HistoryOperation.h"
#include "../node/Node.h"

namespace AxiomModel {

    class SizeNodeOperation : public HistoryOperation {
    public:
        SizeNodeOperation(Project *project, NodeRef nodeRef, QPoint beforeTopLeft, QPoint beforeBottomRight,
                          QPoint afterTopLeft, QPoint afterBottomRight);

        static std::unique_ptr<SizeNodeOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        NodeRef nodeRef;
        QPoint beforeTopLeft;
        QPoint beforeBottomRight;
        QPoint afterTopLeft;
        QPoint afterBottomRight;
    };

}
