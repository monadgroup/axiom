#pragma once

#include <vendor/vst/public.sdk/source/vst2.x/audioeffectx.h>
#include "AxiomApplication.h"
#include "compiler/runtime/Runtime.h"
#include "model/Project.h"
#include "AxiomVstEditor.h"

class AxiomVstEditor;

class AxiomVstPlugin : public AudioEffectX {
public:
    MaximRuntime::Runtime runtime;

    explicit AxiomVstPlugin(audioMasterCallback audioMaster);

    void open() override;

    void suspend() override;

    void resume() override;

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) override;

    VstInt32 processEvents(VstEvents *ev) override;

    void setProgramName(char *name) override;

    void getProgramName(char *name) override;

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

private:

    AxiomVstEditor _editor;
    std::unique_ptr<QByteArray> saveBuffer;

};
