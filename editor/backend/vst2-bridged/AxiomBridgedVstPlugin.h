#pragma once

#include <QtCore/QProcess>
#include <QtCore/QSharedMemory>
#include <public.sdk/source/vst2.x/audioeffectx.h>

#include "Dispatcher.h"
#include "VstChannel.h"

class AxiomBridgedVstPlugin : public AudioEffectX {
public:
    QString channelMemKey;
    QSharedMemory channelMem;
    AxiomBackend::VstChannel &channel;
    AxiomBackend::VstChannel::SeparateData sep;
    QProcess appProcess;
    AxiomBackend::Dispatcher<AxiomBackend::VstChannel::GuiAppToVstQueue> guiDispatcher;
    AxiomBackend::Dispatcher<AxiomBackend::VstChannel::AudioAppToVstQueue> audioDispatcher;

    explicit AxiomBridgedVstPlugin(audioMasterCallback audioMaster);

    ~AxiomBridgedVstPlugin();

    void suspend() override;

    void resume() override;

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) override;

    VstInt32 processEvents(VstEvents *events) override;

    void setSampleRate(float sampleRate) override;

    void setParameter(VstInt32 index, float value) override;

    float getParameter(VstInt32 index) override;

    void getParameterLabel(VstInt32 index, char *label) override;

    void getParameterDisplay(VstInt32 index, char *text) override;

    void getParameterName(VstInt32 index, char *text) override;

    VstInt32 getChunk(void **data, bool isPreset) override;

    VstInt32 setChunk(void *data, VstInt32 byteSize, bool isPreset) override;

    bool getEffectName(char *name) override;

    bool getVendorString(char *text) override;

    bool getProductString(char *text) override;

    VstInt32 getVendorVersion() override;

    VstPlugCategory getPlugCategory() override;

    VstInt32 canDo(char *text) override;

    VstInt32 getNumMidiInputChannels() override;

    bool canParameterBeAutomated(VstInt32 index) override;

private:
    QSharedMemory ioBuffer;
    QSharedMemory serializeBuffer;

    float currentBpm = 0;
    size_t inputCount = 0;
    size_t outputCount = 0;
    size_t expectedInputCount = 0;
    size_t expectedOutputCount = 0;

    float *getInputBufferPtr(size_t inputIndex);
    float *getOutputBufferPtr(size_t outputIndex);

    void handleSetParameter(AxiomBackend::AppGuiSetParameterMessage message);

    void handleUpdateIo(AxiomBackend::AppGuiUpdateIoMessage message);

    AxiomBackend::DispatcherHandlerResult dispatchGuiMessage(AxiomBackend::AppGuiMessage message);

    AxiomBackend::DispatcherHandlerResult dispatchAudioMessage(AxiomBackend::AppAudioMessage message);
};
