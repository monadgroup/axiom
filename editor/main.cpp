#include <QtCore/QFile>

#include "widgets/windows/MainWindow.h"
#include "widgets/surface/NodeSurfacePanel.h"
#include "AxiomApplication.h"
#include "StandaloneAudio.h"

int main(int argc, char *argv[]) {
    auto project = std::make_unique<AxiomModel::Project>();
    AxiomGui::MainWindow window(std::move(project));
    //AxiomStandalone::startupAudio();
    window.show();
    auto result = AxiomApplication::main.exec();
    AxiomStandalone::shutdownAudio();
    return result;
}
