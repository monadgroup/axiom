#include <QtCore/QProcess>
#include <QtCore/QSharedMemory>
#include <QtCore/QCoreApplication>
#include <thread>
#include <iostream>

#include "IdBuffer.h"
#include "VstChannel.h"
#include "Dispatcher.h"

using namespace AxiomBackend;

int main() {
    auto argCount = 0;
    QCoreApplication app(argCount, nullptr);

    auto key = getBufferStringKey(generateNewBufferId());
    std::cout << "Opening shared memory with key " << key.toStdString() << std::endl;
    QSharedMemory channelBuffer(key);
    auto createSuccess = channelBuffer.create(sizeof(VstChannel));
    assert(createSuccess);
    auto channel = ::new (channelBuffer.data()) VstChannel;

    VstChannel::SeparateData sep(key.toStdString());

    Dispatcher<VstChannel::GuiAppToVstQueue> guiDispatcher(channel->guiAppToVst, [channel, &sep](const AppGuiMessage &message) {
        std::cout << "Received message from GUI dispatcher" << std::endl;
        return DispatcherHandlerResult::CONTINUE;
    });

    Dispatcher<VstChannel::AudioAppToVstQueue> audioDispatcher(channel->audioAppToVst, [](const AppAudioMessage &message) {
        std::cout << "Received message from audio dispatcher" << std::endl;
        return DispatcherHandlerResult::CONTINUE;
    });

    QString appProcessPath = "axiom_vst2_bridge.exe";
    QStringList appProcessParams;
    appProcessParams.push_back(key);
    appProcessParams.push_back("instrument");
    std::cout << "Starting process with command line:" << std::endl << "  " << appProcessPath.toStdString();
    for (const auto &param : appProcessParams) {
        std::cout << " " << param.toStdString();
    }
    std::cout << std::endl;

    QProcess appProcess;
    appProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    appProcess.start(appProcessPath, appProcessParams);

    QObject::connect(&appProcess, &QProcess::started,
                     []() {
                         std::cout << "Process started" << std::endl;
                     });
    QObject::connect(&appProcess, qOverload<int>(&QProcess::finished),
                     [](int exitCode) {
                         std::cout << "Process finished with exit code " << exitCode << std::endl;
                     });
    QObject::connect(&appProcess, &QProcess::errorOccurred,
                     [&appProcess](QProcess::ProcessError error) {
        std::cout << "Process errored: " << appProcess.errorString().toStdString() << std::endl;
    });

    std::thread audioThread([&audioDispatcher, &sep]() {
        std::cout << "Running audio dispatcher" << std::endl;
        audioDispatcher.run(sep.audioAppToVstData);
        std::cout << "Audio dispatcher has exited" << std::endl;
    });

    std::thread guiThread([&guiDispatcher, &sep]() {
        std::cout << "Running GUI dispatcher" << std::endl;
        guiDispatcher.run(sep.guiAppToVstData);
        std::cout << "GUI dispatcher has exited" << std::endl;
    });

    return app.exec();
}
