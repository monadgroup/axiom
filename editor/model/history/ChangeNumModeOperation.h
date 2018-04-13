#pragma once

#include "HistoryOperation.h"
#include "../control/NodeNumControl.h"

namespace AxiomModel {

    class ChangeNumModeOperation : public HistoryOperation {
    public:
        ChangeNumModeOperation(Project *project, ControlRef controlRef, NodeNumControl::Mode beforeMode, NodeNumControl::Mode afterMode);

        static std::unique_ptr<ChangeNumModeOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        ControlRef controlRef;
        NodeNumControl::Mode beforeMode;
        NodeNumControl::Mode afterMode;
    };

}
