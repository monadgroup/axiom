#include "RenameNodeOperation.h"

#include "../Project.h"

using namespace AxiomModel;

RenameNodeOperation::RenameNodeOperation(AxiomModel::Project *project, NodeRef nodeRef, QString beforeName,
                                         QString afterName)
    : HistoryOperation(false, HistoryOperation::Type::RENAME_NODE, true), project(project), nodeRef(nodeRef), beforeName(beforeName), afterName(afterName) {

}

std::unique_ptr<RenameNodeOperation> RenameNodeOperation::deserialize(QDataStream &stream,
                                                                      AxiomModel::Project *project) {
    NodeRef nodeRef; stream << nodeRef;
    QString beforeName; stream << beforeName;
    QString afterName; stream << afterName;

    return std::make_unique<RenameNodeOperation>(project, nodeRef, beforeName, afterName);
}

void RenameNodeOperation::forward() {
    auto node = project->findNode(nodeRef);
    node->setNameNoOp(afterName);
}

void RenameNodeOperation::backward() {
    auto node = project->findNode(nodeRef);
    node->setNameNoOp(beforeName);
}

void RenameNodeOperation::serialize(QDataStream &stream) const {
    stream << nodeRef;
    stream << beforeName;
    stream << afterName;
}
