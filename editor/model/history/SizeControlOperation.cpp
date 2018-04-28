#include "SizeControlOperation.h"

#include "../Project.h"

using namespace AxiomModel;

SizeControlOperation::SizeControlOperation(AxiomModel::Project *project, AxiomModel::ControlRef controlRef,
                                           QPoint beforeTopLeft, QPoint beforeBottomRight, QPoint afterTopLeft,
                                           QPoint afterBottomRight)
    : HistoryOperation(false, HistoryOperation::ReadLevel::CONTROL, Type::SIZE_CONTROL, false), project(project), controlRef(std::move(controlRef)),
      beforeTopLeft(beforeTopLeft), beforeBottomRight(beforeBottomRight), afterTopLeft(afterTopLeft),
      afterBottomRight(afterBottomRight) {

}

std::unique_ptr<SizeControlOperation> SizeControlOperation::deserialize(QDataStream &stream,
                                                                        AxiomModel::Project *project) {
    ControlRef controlRef; stream >> controlRef;
    QPoint beforeTopLeft; stream >> beforeTopLeft;
    QPoint beforeBottomRight; stream >> beforeBottomRight;
    QPoint afterTopLeft; stream >> afterTopLeft;
    QPoint afterBottomRight; stream >> afterBottomRight;

    return std::make_unique<SizeControlOperation>(project, controlRef, beforeTopLeft, beforeBottomRight, afterTopLeft, afterBottomRight);
}

void SizeControlOperation::forward() {
    auto control = project->findControl(controlRef);
    control->setCorners(afterTopLeft, afterBottomRight);
}

void SizeControlOperation::backward() {
    auto control = project->findControl(controlRef);
    control->setCorners(beforeTopLeft, beforeBottomRight);
}

void SizeControlOperation::serialize(QDataStream &stream) const {
    stream << controlRef;
    stream << beforeTopLeft;
    stream << beforeBottomRight;
    stream << afterTopLeft;
    stream << afterBottomRight;
}
