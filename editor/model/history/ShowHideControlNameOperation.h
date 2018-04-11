#pragma once

#include "HistoryOperation.h"
#include "../control/NodeControl.h"

namespace AxiomModel {

    class ShowHideControlNameOperation : public HistoryOperation {
    public:
        ShowHideControlNameOperation(Project *project, ControlRef controlRef, bool show);

        static std::unique_ptr<ShowHideControlNameOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        ControlRef controlRef;
        bool show;
    };

}
