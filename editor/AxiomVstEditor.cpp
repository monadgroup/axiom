#include "AxiomVstEditor.h"

#include "AxiomVstPlugin.h"

AxiomVstEditor::AxiomVstEditor(AxiomVstPlugin *plugin) : plugin(plugin) {
    window.loadProject(&plugin->project);
}

bool AxiomVstEditor::open(void *ptr) {
    AEffEditor::open(ptr);
    window.show();
}

void AxiomVstEditor::close() {
    window.hide();
    AEffEditor::close();
}

void AxiomVstEditor::idle() {
    AxiomApplication::main->processEvents();
    AxiomApplication::main->sendPostedEvents(&window);
}
