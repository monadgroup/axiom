#include "AxiomVstEditor.h"

AxiomVstEditor::AxiomVstEditor(AxiomApplication *application, AxiomBackend::AudioBackend *backend)
    : editor(application, backend) {}

bool AxiomVstEditor::open(void *ptr) {
    AEffEditor::open(ptr);
    editor.show();
    return true;
}

void AxiomVstEditor::close() {
    editor.hide();
    AEffEditor::close();
}

void AxiomVstEditor::idle() {
    editor.idle();
}
