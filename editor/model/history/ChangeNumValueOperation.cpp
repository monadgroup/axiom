#include "ChangeNumValueOperation.h"

#include "../Project.h"

using namespace AxiomModel;

ChangeNumValueOperation::ChangeNumValueOperation(AxiomModel::Project *project, AxiomModel::ControlRef controlRef,
                                                 MaximRuntime::NumValue beforeVal, MaximRuntime::NumValue afterVal)
    : HistoryOperation(false, HistoryOperation::ReadLevel::CONTROL, HistoryOperation::Type::CHANGE_NUM_VAL, false), project(project),
      controlRef(std::move(controlRef)), beforeVal(beforeVal), afterVal(afterVal) {

}

std::unique_ptr<ChangeNumValueOperation> ChangeNumValueOperation::deserialize(QDataStream &stream,
                                                                              AxiomModel::Project *project) {
    ControlRef controlRef; stream >> controlRef;
    MaximRuntime::NumValue beforeVal; stream >> beforeVal;
    MaximRuntime::NumValue afterVal; stream >> afterVal;

    return std::make_unique<ChangeNumValueOperation>(project, controlRef, beforeVal, afterVal);
}

void ChangeNumValueOperation::forward() {
    auto numControl = dynamic_cast<NodeNumControl*>(project->findControl(controlRef));
    assert(numControl);

    numControl->setValue(afterVal);
}

void ChangeNumValueOperation::backward() {
    auto numControl = dynamic_cast<NodeNumControl*>(project->findControl(controlRef));
    assert(numControl);

    numControl->setValue(beforeVal);
}

void ChangeNumValueOperation::serialize(QDataStream &stream) const {
    stream << controlRef;
    stream << beforeVal;
    stream << afterVal;
}
