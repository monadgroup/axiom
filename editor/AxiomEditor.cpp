#include "AxiomEditor.h"

#include "AxiomApplication.h"
#include "backend/AudioBackend.h"
#include "model/Project.h"

AxiomEditor::AxiomEditor(AxiomApplication *, AxiomBackend::AudioBackend *backend) : _window(backend) {
    backend->setEditor(this);
    _window.setProject(std::make_unique<AxiomModel::Project>(backend->createDefaultConfiguration()));
    _window.importLibraryFrom(":/default.axl");
}

int AxiomEditor::run() {
    _window.show();
    return QApplication::exec();
}

void AxiomEditor::show() {
    _window.show();
}

void AxiomEditor::hide() {
    _window.hide();
}

void AxiomEditor::idle() {
    QApplication::processEvents();
    QApplication::sendPostedEvents(&_window);
}
