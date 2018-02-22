#pragma once

#include <vendor/vst/public.sdk/source/vst2.x/aeffeditor.h>
#include "widgets/windows/MainWindow.h"

class AxiomVstPlugin;

class AxiomVstEditor : public AEffEditor {
public:
    explicit AxiomVstEditor(AxiomVstPlugin *plugin);

    bool open(void *ptr) override;

    void close() override;

    void idle() override;

private:

    AxiomVstPlugin *plugin;

    AxiomGui::MainWindow window;

};
