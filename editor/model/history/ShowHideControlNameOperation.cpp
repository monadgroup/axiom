#include "ShowHideControlNameOperation.h"
#include <utility>
#include "../Project.h"

using namespace AxiomModel;

ShowHideControlNameOperation::ShowHideControlNameOperation(AxiomModel::Project *project, ControlRef controlRef,
                                                           bool show)
    : HistoryOperation(false, HistoryOperation::ReadLevel::CONTROL, HistoryOperation::Type::SHOW_HIDE_CONTROL_NAME), project(project), controlRef(
    std::move(controlRef)), show(show) {

}

std::unique_ptr<ShowHideControlNameOperation> ShowHideControlNameOperation::deserialize(QDataStream &stream,
                                                                                        AxiomModel::Project *project) {
    ControlRef controlRef; stream >> controlRef;
    bool show; stream >> show;

    return std::make_unique<ShowHideControlNameOperation>(project, controlRef, show);
}

void ShowHideControlNameOperation::forward() {
    auto control = project->findControl(controlRef);
    control->setShowNameNoOp(show);
}

void ShowHideControlNameOperation::backward() {
    auto control = project->findControl(controlRef);
    control->setShowNameNoOp(!show);
}

void ShowHideControlNameOperation::serialize(QDataStream &stream) const {
    stream << controlRef;
    stream << show;
}
