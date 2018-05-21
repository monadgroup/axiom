#pragma once

#include <vendor/vst/public.sdk/source/vst2.x/aeffeditor.h>
#include "widgets/windows/MainWindow.h"

class AxiomVstPlugin;

class AxiomVstEditor : public AEffEditor {
public:
    explicit AxiomVstEditor(MaximRuntime::Runtime *runtime, std::unique_ptr<AxiomModel::Project> project);

    AxiomModel::Project *project() const;
    
    void setProject(std::unique_ptr<AxiomModel::Project> project);
    
    bool open(void *ptr) override;

    void close() override;

    void idle() override;

private:

    AxiomGui::MainWindow window;

};
