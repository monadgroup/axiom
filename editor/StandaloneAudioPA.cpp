#include "StandaloneAudio.h"

#include <portaudio.h>
#include <cassert>
#include <mutex>

#include "compiler/runtime/Runtime.h"

static PaStream *stream;

static int paCallback(const void *, void *outputBuffer, unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                      void *userData) {
    auto runtime = (MaximRuntime::Runtime *) userData;
    std::lock_guard<std::mutex> lock(runtime->mutex());

    auto outputPtr = runtime->mainSurface()->output->control()->group()->currentValuePtr();
    assert(outputPtr);

    auto outputNums = (float*) outputBuffer;

    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        runtime->generate();
        auto outputNum = runtime->op().readNum(outputPtr);
        *outputNums++ = outputNum.left;
        *outputNums++ = outputNum.right;
    }
}

void AxiomStandalone::startupAudio(MaximRuntime::Runtime *runtime) {
    PaError err;
    Pa_Initialize();
    assert(err == paNoError);

    err = Pa_OpenDefaultStream(&stream,
                               0, // no inputs
                               2, // stereo output
                               paFloat32,
                               44100,
                               paFramesPerBufferUnspecified,
                               paCallback,
                               runtime);
    assert(err == paNoError);

    err = Pa_StartStream(stream);
    assert(err == paNoError);
}

void AxiomStandalone::shutdownAudio() {
    PaError err;
    err = Pa_StopStream(stream);
    assert(err == paNoError);

    err = Pa_CloseStream(stream);
    assert(err == paNoError);

    err = Pa_Terminate();
    assert(err == paNoError);
}
