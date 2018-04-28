#pragma once

#include "HistoryOperation.h"
#include "../Ref.h"

namespace AxiomModel {

    class ChangeCodeOperation : public HistoryOperation {
    public:
        ChangeCodeOperation(Project *project, NodeRef nodeRef, QString beforeCode, QString afterCode);

        static std::unique_ptr<ChangeCodeOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        NodeRef nodeRef;
        QString beforeCode;
        QString afterCode;
    };

}
