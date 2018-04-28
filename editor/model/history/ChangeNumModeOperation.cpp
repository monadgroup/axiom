#include "ChangeNumModeOperation.h"

#include "../Project.h"

using namespace AxiomModel;

ChangeNumModeOperation::ChangeNumModeOperation(Project *project, ControlRef controlRef,
                                               NodeNumControl::Mode beforeMode, NodeNumControl::Mode afterMode)
    : HistoryOperation(false, HistoryOperation::ReadLevel::CONTROL, HistoryOperation::Type::CHANGE_NUM_MODE), project(project),
      controlRef(std::move(controlRef)), beforeMode(beforeMode), afterMode(afterMode) {

}

std::unique_ptr<ChangeNumModeOperation> ChangeNumModeOperation::deserialize(QDataStream &stream,
                                                                            AxiomModel::Project *project) {
    ControlRef controlRef; stream >> controlRef;
    uint8_t beforeModeInt; stream >> beforeModeInt;
    uint8_t afterModeInt; stream >> afterModeInt;

    return std::make_unique<ChangeNumModeOperation>(project, controlRef, (NodeNumControl::Mode) beforeModeInt,
                                                    (NodeNumControl::Mode) afterModeInt);
}

void ChangeNumModeOperation::forward() {
    auto numControl = dynamic_cast<NodeNumControl *>(project->findControl(controlRef));
    assert(numControl);

    numControl->setModeNoOp(afterMode);
}

void ChangeNumModeOperation::backward() {
    auto numControl = dynamic_cast<NodeNumControl *>(project->findControl(controlRef));
    assert(numControl);

    numControl->setModeNoOp(beforeMode);
}

void ChangeNumModeOperation::serialize(QDataStream &stream) const {
    stream << controlRef;
    stream << (uint8_t) beforeMode;
    stream << (uint8_t) afterMode;
}
