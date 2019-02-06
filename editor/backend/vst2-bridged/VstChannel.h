#pragma once

#include "SharedQueue.h"
#include "VstMessage.h"
#include "AppMessage.h"

namespace AxiomBackend {

    struct VstChannel {
        static constexpr size_t QueueSize = 256;

        using GuiVstToAppQueue = Queue<VstGuiMessage, QueueSize>;
        using GuiAppToVstQueue = Queue<AppGuiMessage, QueueSize>;

        using AudioVstToAppQueue = Queue<VstAudioMessage, QueueSize>;
        using AudioAppToVstQueue = Queue<AppAudioMessage, QueueSize>;

        class SeparateData {
        public:
            GuiVstToAppQueue::SeparateData guiVstToAppData;
            GuiAppToVstQueue::SeparateData guiAppToVstData;

            AudioVstToAppQueue::SeparateData audioVstToAppData;
            AudioAppToVstQueue::SeparateData audioAppToVstData;

            explicit SeparateData(const std::string &id);
        };

        GuiVstToAppQueue guiVstToApp;
        GuiAppToVstQueue guiAppToVst;

        AudioVstToAppQueue audioVstToApp;
        AudioAppToVstQueue audioAppToVst;
    };

}
