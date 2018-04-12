#pragma once

#include "HistoryOperation.h"
#include "../control/NodeControl.h"

namespace AxiomModel {

    class ConnectControlsOperation : public HistoryOperation {
    public:
        ConnectControlsOperation(Project *project, ControlRef controlARef, ControlRef controlBRef);

        static std::unique_ptr<ConnectControlsOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        ControlRef controlARef;
        ControlRef controlBRef;

    };

}
