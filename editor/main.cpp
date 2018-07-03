#include <QtCore/QFile>

#include "widgets/windows/MainWindow.h"
#include "widgets/surface/NodeSurfacePanel.h"
#include "AxiomApplication.h"
#include "StandaloneAudio.h"
#include "compiler/interface/Frontend.h"
#include "compiler/interface/Runtime.h"
#include "model/objects/RootSurface.h"

int main(int argc, char *argv[]) {
    MaximFrontend::maxim_initialize();
    MaximCompiler::Runtime runtime;

    auto project = std::make_unique<AxiomModel::Project>();
    project->rootSurface()->attachRuntime(&runtime);

    AxiomGui::MainWindow window(std::move(project));
    //AxiomStandalone::startupAudio();
    window.show();
    auto result = AxiomApplication::main.exec();
    AxiomStandalone::shutdownAudio();
    return result;
}
