#ifndef AXIOM_INSTRUMENT_H
#define AXIOM_INSTRUMENT_H

#include "AxiomCommon.h"

#define AXIOM_SAMPLERATE 44100
#define AXIOM_BPM 60

#define AXIOM_INPUT_PORTAL 0
#define AXIOM_OUTPUT_PORTAL 1

#ifdef __cplusplus
extern "C" {
#endif
void __cdecl axiom_init();
void __cdecl axiom_packup();
void __cdecl axiom_generate();

void *__cdecl axiom_get_portal(uint32_t id);

void __cdecl axiom_midi_push(AxiomMidi *midi, AxiomMidiEvent event);
#ifdef __cplusplus
}
#endif

#endif
