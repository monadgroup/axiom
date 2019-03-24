#pragma once

#include <optional>
#include <public.sdk/source/vst2.x/aeffeditor.h>

#include "../../AxiomEditor.h"

class AxiomVstPlugin;

class AxiomVstEditor : public AEffEditor {
public:
    explicit AxiomVstEditor(AxiomApplication *application, AxiomVstPlugin *plugin);

    bool getRect(ERect **rect) override;

    bool open(void *ptr) override;

    void close() override;

    void idle() override;

    void resize(QSize newSize);

private:
    ERect editorSize;
    AxiomEditor editor;
    AxiomVstPlugin *plugin;
};
