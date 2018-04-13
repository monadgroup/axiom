#pragma once

#include "HistoryOperation.h"
#include "../control/NodeControl.h"
#include "compiler/runtime/ValueOperator.h"

namespace AxiomModel {

    class ChangeNumValueOperation : public HistoryOperation {
    public:
        ChangeNumValueOperation(Project *project, ControlRef controlRef, MaximRuntime::NumValue beforeVal, MaximRuntime::NumValue afterVal);

        static std::unique_ptr<ChangeNumValueOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        ControlRef controlRef;
        MaximRuntime::NumValue beforeVal;
        MaximRuntime::NumValue afterVal;
    };

}
