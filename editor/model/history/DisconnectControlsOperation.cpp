#include "DisconnectControlsOperation.h"

#include "../Project.h"
#include "compiler/runtime/Control.h"

using namespace AxiomModel;

DisconnectControlsOperation::DisconnectControlsOperation(AxiomModel::Project *project,
                                                         AxiomModel::ControlRef controlARef,
                                                         AxiomModel::ControlRef controlBRef)
    : HistoryOperation(true, HistoryOperation::ReadLevel::CONTROL, HistoryOperation::Type::DISCONNECT_CONTROLS), project(project),
      controlARef(std::move(controlARef)), controlBRef(std::move(controlBRef)) {

}

std::unique_ptr<DisconnectControlsOperation> DisconnectControlsOperation::deserialize(QDataStream &stream,
                                                                                      AxiomModel::Project *project) {
    ControlRef controlARef; stream >> controlARef;
    ControlRef controlBRef; stream >> controlBRef;

    return std::make_unique<DisconnectControlsOperation>(project, controlARef, controlBRef);
}

void DisconnectControlsOperation::forward() {
    auto controlA = project->findControl(controlARef);
    auto controlB = project->findControl(controlBRef);

    auto wire = controlA->sink()->getConnectingWire(controlB->sink());
    if (wire) wire->removeNoOp();
}

void DisconnectControlsOperation::backward() {
    auto controlA = project->findControl(controlARef);
    auto controlB = project->findControl(controlBRef);
    assert(controlA->node->parentSchematic == controlB->node->parentSchematic);

    controlA->node->parentSchematic->connectSinks(controlA->sink(), controlB->sink());
}

void DisconnectControlsOperation::serialize(QDataStream &stream) const {
    stream << controlARef;
    stream << controlBRef;
}
