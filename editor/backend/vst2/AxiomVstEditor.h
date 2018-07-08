#pragma once

#include <public.sdk/source/vst2.x/aeffeditor.h>

#include "../../AxiomEditor.h"

class AxiomEditor;

class AxiomVstEditor : public AEffEditor {
public:
    explicit AxiomVstEditor(AxiomBackend::AudioBackend *backend);

    bool open(void *ptr) override;

    void close() override;

    void idle() override;

private:
    AxiomEditor editor;
};
