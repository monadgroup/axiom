#include "AxiomEditor.h"

#include "AxiomApplication.h"
#include "backend/AudioBackend.h"
#include "model/Project.h"

AxiomEditor::AxiomEditor(AxiomBackend::AudioBackend *backend) {
    backend->setEditor(this);
    _window.setProject(std::make_unique<AxiomModel::Project>(backend->createDefaultConfiguration()));
    _window.importLibraryFrom(":/default.axl");
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
