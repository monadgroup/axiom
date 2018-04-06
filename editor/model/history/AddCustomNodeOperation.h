#pragma once

#include "HistoryOperation.h"
#include "../Project.h"

namespace AxiomModel {

    class AddCustomNodeOperation : public HistoryOperation {
    public:
        AddCustomNodeOperation(Project *project, SurfaceRef surfaceRef, QString name, QPoint pos);

        void forward() override;

        void backward() override;

    private:

        Project *project;
        SurfaceRef surfaceRef;
        QString name;
        QPoint pos;
        size_t insertIndex;
    };

}
