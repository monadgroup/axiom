#include "ChangeCodeOperation.h"

#include "../Project.h"
#include "../node/CustomNode.h"

using namespace AxiomModel;

ChangeCodeOperation::ChangeCodeOperation(AxiomModel::Project *project, NodeRef nodeRef, QString beforeCode,
                                         QString afterCode)
    : HistoryOperation(true, HistoryOperation::ReadLevel::NODE, HistoryOperation::Type::CHANGE_CODE, false), project(project), nodeRef(std::move(nodeRef)),
      beforeCode(std::move(beforeCode)), afterCode(std::move(afterCode)) {

}

std::unique_ptr<ChangeCodeOperation> ChangeCodeOperation::deserialize(QDataStream &stream,
                                                                      AxiomModel::Project *project) {
    NodeRef nodeRef; stream >> nodeRef;
    QString beforeCode; stream >> beforeCode;
    QString afterCode; stream >> afterCode;

    return std::make_unique<ChangeCodeOperation>(project, nodeRef, beforeCode, afterCode);
}

void ChangeCodeOperation::forward() {
    auto customNode = dynamic_cast<CustomNode *>(project->findNode(nodeRef));
    assert(customNode);

    customNode->setCode(afterCode);
}

void ChangeCodeOperation::backward() {
    auto customNode = dynamic_cast<CustomNode *>(project->findNode(nodeRef));
    assert(customNode);

    customNode->setCode(beforeCode);
}

void ChangeCodeOperation::serialize(QDataStream &stream) const {
    stream << nodeRef;
    stream << beforeCode;
    stream << afterCode;
}
