#include "AxiomEditor.h"

#include "AxiomApplication.h"
#include "backend/AudioBackend.h"
#include "model/Project.h"

static std::unique_ptr<AxiomModel::Project> createProject(MaximCompiler::Runtime *runtime,
                                                          AxiomBackend::AudioBackend *backend) {
    auto project = std::make_unique<AxiomModel::Project>(backend->createDefaultConfiguration());
    project->mainRoot().attachRuntime(runtime);
    return std::move(project);
}

AxiomEditor::AxiomEditor(AxiomBackend::AudioBackend *backend)
    : _runtime(true, false, AxiomApplication::main.jit()), _window(createProject(&_runtime, backend)) {
    backend->setEditor(this);
    _window.project()->mainRoot().attachBackend(backend);
    backend->internalUpdateConfiguration();
}

int AxiomEditor::run() {
    _window.show();
    return AxiomApplication::main.exec();
}

void AxiomEditor::show() {
    _window.show();
}

void AxiomEditor::hide() {
    _window.hide();
}

void AxiomEditor::idle() {
    AxiomApplication::main.processEvents();
    AxiomApplication::main.sendPostedEvents(&_window);
}
