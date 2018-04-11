#pragma once

#include "HistoryOperation.h"
#include "../control/NodeControl.h"

namespace AxiomModel {

    class SizeControlOperation : public HistoryOperation {
    public:
        SizeControlOperation(Project *project, ControlRef controlRef, QPoint beforeTopLeft, QPoint beforeBottomRight,
                             QPoint afterTopLeft, QPoint afterBottomRight);

        static std::unique_ptr<SizeControlOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        ControlRef controlRef;
        QPoint beforeTopLeft;
        QPoint beforeBottomRight;
        QPoint afterTopLeft;
        QPoint afterBottomRight;
    };

}
