#pragma once

#include "HistoryOperation.h"
#include "../control/NodeControl.h"

namespace AxiomModel {

    class MoveControlOperation : public HistoryOperation {
    public:
        MoveControlOperation(Project *project, ControlRef controlRef, QPoint startPos, QPoint endPos);

        static std::unique_ptr<MoveControlOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        ControlRef controlRef;
        QPoint startPos;
        QPoint endPos;
    };

}
