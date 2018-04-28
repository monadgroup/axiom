#include "MoveNodeOperation.h"

#include "../Project.h"

using namespace AxiomModel;

MoveNodeOperation::MoveNodeOperation(AxiomModel::Project *project, AxiomModel::NodeRef nodeRef, QPoint startPos,
                                     QPoint endPos)
    : HistoryOperation(false, HistoryOperation::ReadLevel::NODE, Type::MOVE_NODE, false), project(project), nodeRef(std::move(nodeRef)),
      startPos(startPos),
      endPos(endPos) {

}

std::unique_ptr<MoveNodeOperation> MoveNodeOperation::deserialize(QDataStream &stream, AxiomModel::Project *project) {
    NodeRef nodeRef;
    stream >> nodeRef;
    QPoint startPos;
    stream >> startPos;
    QPoint endPos;
    stream >> endPos;

    return std::make_unique<MoveNodeOperation>(project, nodeRef, startPos, endPos);
}


void MoveNodeOperation::forward() {
    auto node = project->findNode(nodeRef);
    node->setPos(endPos);
}

void MoveNodeOperation::backward() {
    auto node = project->findNode(nodeRef);
    node->setPos(startPos);
}

void MoveNodeOperation::serialize(QDataStream &stream) const {
    stream << nodeRef;
    stream << startPos;
    stream << endPos;
}
