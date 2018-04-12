#include "ConnectControlsOperation.h"

#include "../Project.h"
#include "compiler/runtime/Control.h"

using namespace AxiomModel;

ConnectControlsOperation::ConnectControlsOperation(AxiomModel::Project *project, AxiomModel::ControlRef controlARef,
                                                   AxiomModel::ControlRef controlBRef)
    : HistoryOperation(true, HistoryOperation::Type::CONNECT_CONTROLS), project(project), controlARef(controlARef), controlBRef(controlBRef) {

}

std::unique_ptr<ConnectControlsOperation> ConnectControlsOperation::deserialize(QDataStream &stream,
                                                                                AxiomModel::Project *project) {
    ControlRef controlARef; stream >> controlARef;
    ControlRef controlBRef; stream >> controlBRef;

    return std::make_unique<ConnectControlsOperation>(project, controlARef, controlBRef);
}

void ConnectControlsOperation::forward() {
    auto controlA = project->findControl(controlARef);
    auto controlB = project->findControl(controlBRef);
    assert(controlA->node->parentSchematic == controlB->node->parentSchematic);

    controlA->node->parentSchematic->connectSinks(controlA->sink(), controlB->sink());
}

void ConnectControlsOperation::backward() {
    auto controlA = project->findControl(controlARef);
    auto controlB = project->findControl(controlBRef);

    auto wire = controlA->sink()->getConnectingWire(controlB->sink());
    if (wire) {
        wire->remove();
    }
}

void ConnectControlsOperation::serialize(QDataStream &stream) const {
    stream << controlARef;
    stream << controlBRef;
}
