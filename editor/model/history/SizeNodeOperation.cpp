#include "SizeNodeOperation.h"

#include "../Project.h"

using namespace AxiomModel;

SizeNodeOperation::SizeNodeOperation(AxiomModel::Project *project, AxiomModel::NodeRef nodeRef, QPoint beforeTopLeft, QPoint beforeBottomRight, QPoint afterTopLeft, QPoint afterBottomRight)
    : HistoryOperation(false, Type::SIZE_NODE, false), project(project), nodeRef(std::move(nodeRef)), beforeTopLeft(beforeTopLeft), beforeBottomRight(beforeBottomRight), afterTopLeft(afterTopLeft), afterBottomRight(afterBottomRight) {

}

std::unique_ptr<SizeNodeOperation> SizeNodeOperation::deserialize(QDataStream &stream, AxiomModel::Project *project) {
    NodeRef nodeRef; stream >> nodeRef;
    QPoint beforeTopLeft; stream >> beforeTopLeft;
    QPoint beforeBottomRight; stream >> beforeBottomRight;
    QPoint afterTopLeft; stream >> afterTopLeft;
    QPoint afterBottomRight; stream >> afterBottomRight;

    return std::make_unique<SizeNodeOperation>(project, nodeRef, beforeTopLeft, beforeBottomRight, afterTopLeft, afterBottomRight);
}

void SizeNodeOperation::forward() {
    auto node = project->findNode(nodeRef);
    node->setCorners(afterTopLeft, afterBottomRight);
}

void SizeNodeOperation::backward() {
    auto node = project->findNode(nodeRef);
    node->setCorners(beforeTopLeft, beforeBottomRight);
}

void SizeNodeOperation::serialize(QDataStream &stream) const {
    stream << nodeRef;
    stream << beforeTopLeft;
    stream << beforeBottomRight;
    stream << afterTopLeft;
    stream << afterBottomRight;
}
