#pragma once

#include <QtCore/QByteArray>
#include <public.sdk/source/vst2.x/audioeffectx.h>

#include "AxiomVstEditor.h"
#include "VstAudioBackend.h"

class AxiomVstPlugin : public AudioEffectX {
public:
    explicit AxiomVstPlugin(audioMasterCallback audioMaster);

    ~AxiomVstPlugin() override;

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

    void backendSetParameter(size_t parameter, AxiomBackend::NumValue value);

private:
    VstAudioBackend backend;
    AxiomVstEditor editor;
    QByteArray saveBuffer;
};
