#include "AxiomVstEditor.h"

#include "AxiomVstPlugin.h"
#include "widgets/surface/NodeSurfacePanel.h"

AxiomVstEditor::AxiomVstEditor(MaximRuntime::Runtime *runtime, std::unique_ptr<AxiomModel::Project> project) : window(runtime, std::move(project)) {
}

AxiomModel::Project* AxiomVstEditor::project() const {
    return window.project();
}

void AxiomVstEditor::setProject(std::unique_ptr<AxiomModel::Project> project) {
    window.setProject(std::move(project));
}

bool AxiomVstEditor::open(void *ptr) {
    AEffEditor::open(ptr);
    window.show();
    return true;
}

void AxiomVstEditor::close() {
    window.hide();
    AEffEditor::close();
}

void AxiomVstEditor::idle() {
    AxiomApplication::main.processEvents();
    AxiomApplication::main.sendPostedEvents(&window);
}
