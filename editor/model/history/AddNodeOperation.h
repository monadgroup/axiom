#pragma once

#include "HistoryOperation.h"
#include "../Project.h"
#include "../node/Node.h"

namespace AxiomModel {

    class AddNodeOperation : public HistoryOperation {
    public:
        AddNodeOperation(Project *project, SurfaceRef surfaceRef, Node::Type type, QString name, QPoint pos);

        void forward() override;

        void backward() override;

    private:

        Project *project;
        SurfaceRef surfaceRef;
        Node::Type type;
        QString name;
        QPoint pos;
        size_t insertIndex;
    };

}
