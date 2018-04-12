#pragma once

#include "HistoryOperation.h"
#include "../control/NodeControl.h"

namespace AxiomModel {

    class DisconnectControlsOperation : public HistoryOperation {
    public:
        DisconnectControlsOperation(Project *project, ControlRef controlARef, ControlRef controlBRef);

        static std::unique_ptr<DisconnectControlsOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        ControlRef controlARef;
        ControlRef controlBRef;

    };

}
