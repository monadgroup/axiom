#include "MoveControlOperation.h"

#include "../Project.h"

using namespace AxiomModel;

MoveControlOperation::MoveControlOperation(AxiomModel::Project *project, AxiomModel::ControlRef controlRef,
                                           QPoint startPos, QPoint endPos)
    : HistoryOperation(false, HistoryOperation::ReadLevel::CONTROL, Type::MOVE_CONTROL, false), project(project), controlRef(std::move(controlRef)), startPos(startPos), endPos(endPos) {

}

std::unique_ptr<MoveControlOperation> MoveControlOperation::deserialize(QDataStream &stream,
                                                                        AxiomModel::Project *project) {
    ControlRef controlRef; stream >> controlRef;
    QPoint startPos; stream >> startPos;
    QPoint endPos; stream >> endPos;

    return std::make_unique<MoveControlOperation>(project, controlRef, startPos, endPos);
}

void MoveControlOperation::forward() {
    auto control = project->findControl(controlRef);
    control->setPos(endPos);
}

void MoveControlOperation::backward() {
    auto control = project->findControl(controlRef);
    control->setPos(startPos);
}

void MoveControlOperation::serialize(QDataStream &stream) const {
    stream << controlRef;
    stream << startPos;
    stream << endPos;
}
