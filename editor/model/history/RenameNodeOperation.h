#pragma once

#include "HistoryOperation.h"
#include "../node/Node.h"

namespace AxiomModel {

    class RenameNodeOperation : public HistoryOperation {
    public:
        RenameNodeOperation(Project *project, NodeRef nodeRef, QString beforeName, QString afterName);

        static std::unique_ptr<RenameNodeOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;
        
    private:
        
        Project *project;
        NodeRef nodeRef;
        QString beforeName;
        QString afterName;
    };

}
