#pragma once

#include "HistoryOperation.h"
#include "../node/Node.h"

namespace AxiomModel {

    class CreateNodeOperation : public HistoryOperation {
    public:
        CreateNodeOperation(Project *project, NodeRef nodeRef, Node::Type type, QString name, QPoint pos);

        static std::unique_ptr<CreateNodeOperation> deserialize(QDataStream &stream, Project *project);

        void forward() override;

        void backward() override;

        void serialize(QDataStream &stream) const override;

    private:

        Project *project;
        NodeRef nodeRef;
        Node::Type type;
        QString name;
        QPoint pos;
    };

}
