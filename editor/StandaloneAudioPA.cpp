#include "StandaloneAudio.h"

#include <cassert>
#include <iostream>
#include <mutex>
#include <portaudio.h>

static PaStream *stream;

static int paCallback(const void *, void *outputBuffer, unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    /*auto runtime = (MaximRuntime::Runtime *) userData;
    std::lock_guard<std::mutex> lock(runtime->mutex());

    auto outputPtr = runtime->mainSurface()->output->control()->group()->currentValuePtr();
    assert(outputPtr);

    auto outputNums = (float*) outputBuffer;

    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        runtime->generate();
        auto outputNum = runtime->op().readNum(outputPtr);
        *outputNums++ = outputNum.left;
        *outputNums++ = outputNum.right;
    }*/
}

static void checkError(PaError error) {
    if (error != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(error) << std::endl;
        abort();
    }
}

void AxiomStandalone::startupAudio() {
    /*checkError(Pa_Initialize());
    checkError(Pa_OpenDefaultStream(&stream,
                                    0, // no inputs
                                    2, // stereo output
                                    paFloat32,
                                    44100,
                                    paFramesPerBufferUnspecified,
                                    paCallback,
                                    runtime));
    checkError(Pa_StartStream(stream));*/
}

void AxiomStandalone::shutdownAudio() {
    checkError(Pa_StopStream(stream));
    checkError(Pa_CloseStream(stream));
    checkError(Pa_Terminate());
}
